#include <bbt/pollevent/Event.hpp>

#include <bbt/rpc/RpcClient.hpp>
#include <bbt/rpc/detail/Protocol.hpp>

using namespace bbt::core::errcode;
using namespace bbt::network;
using namespace bbt::core;

namespace bbt::rpc
{

RpcClient::RpcClient(std::shared_ptr<bbt::network::EvThread> io_thread):
    m_tcp_client(std::make_shared<bbt::network::TcpClient>(io_thread))
{
    m_update_event = io_thread->RegisterEvent(-1, bbt::pollevent::EventOpt::PERSIST, 
    [this](int fd, short events, EventId id) {
        _Update();
    });

    m_update_event->StartListen(detail::rpc_client_check_timeout);
}

RpcClient::~RpcClient()
{
    m_tcp_client = nullptr;
    m_update_event = nullptr;
    FailedAll();
}

ErrOpt RpcClient::Init(const char* ip, int port, int connect_timeout, int connection_timeout, const std::function<void(std::shared_ptr<RpcClient>)>& on_connect_callback)
{
    m_tcp_client->Init();
    m_tcp_client->SetOnSend([weak_this{weak_from_this()}](ConnId id, ErrOpt err, size_t len) {
        if (auto shared_this = weak_this.lock(); shared_this != nullptr)
            shared_this->OnSend(id, err, len);
    });
    m_tcp_client->SetOnRecv([weak_this{weak_from_this()}](ConnId id, const Buffer& buffer) {
        if (auto shared_this = weak_this.lock(); shared_this != nullptr)
            shared_this->OnRecv(id, buffer);
    });
    m_tcp_client->SetOnClose([weak_this{weak_from_this()}](ConnId id) {
        if (auto shared_this = weak_this.lock(); shared_this != nullptr)
            shared_this->OnClose(id);
    });
    m_tcp_client->SetOnTimeout([weak_this{weak_from_this()}](ConnId id) {
        if (auto shared_this = weak_this.lock(); shared_this != nullptr)
            shared_this->OnTimeout(id);
    });
    m_tcp_client->SetOnErr([weak_this{weak_from_this()}](const Errcode& err) {
        if (auto shared_this = weak_this.lock(); shared_this != nullptr)
            shared_this->OnError(err);
    });
    m_tcp_client->SetConnectionTimeout(connection_timeout);
    m_tcp_client->SetOnConnect([weak_this{weak_from_this()}](ConnId id, ErrOpt err) {
        if (auto shared_this = weak_this.lock(); shared_this != nullptr)
            shared_this->OnConnect(id, err);
    });
    m_on_connect_callback = on_connect_callback;

    return m_tcp_client->AsyncConnect(IPAddress(ip, port), connect_timeout);
}

void RpcClient::OnError(const bbt::core::errcode::Errcode& err)
{
    std::cerr << bbt::core::clock::getnow_str() << "[RpcClient::DefaultErr]" <<  " " << err.What() << std::endl;
}

void RpcClient::OnRecv(ConnId id, const bbt::core::Buffer& buffer)
{
    std::vector<bbt::core::Buffer> protocols;

    {
        std::lock_guard<std::mutex> lock(m_all_opt_mtx);
        m_recv_buffer.WriteString(buffer.Peek(), buffer.Size());

        if (auto err = detail::Helper::ParseProtocolFromBuffer(m_recv_buffer, protocols); err.has_value())
        {
            OnError(err.value());
            return;
        }
    }

    for (auto& protocol : protocols)
    {
        auto* head = (detail::ProtocolHead*)protocol.Peek();
        Buffer body{protocol.Peek() + sizeof(detail::ProtocolHead), protocol.Size() - sizeof(detail::ProtocolHead)};
        if (auto err = OnReply(head->call_seq, body); err.has_value())
        {
            OnError(err.value());
            return;
        }
    }
}

void RpcClient::OnConnect(ConnId id, bbt::core::errcode::ErrOpt err)
{
    if (err.has_value())
    {
        OnError(err.value());
        return;
    }

    {
        std::lock_guard<std::mutex> lock(m_all_opt_mtx);
        m_recv_buffer.Clear();
    }

    if (m_on_connect_callback)
        m_on_connect_callback(shared_from_this());
}

void RpcClient::OnClose(ConnId id)
{
    FailedAll();
}

void RpcClient::FailedAll()
{
    std::lock_guard<std::mutex> lock(m_all_opt_mtx);
    for (auto& it : m_reply_caller_map)
    {
        auto caller = it.second;
        if (caller)
        {
            bbt::core::Buffer buffer{0};
            caller->Reply(buffer, Errcode{BBT_RPC_ERR_PREFIX "[RpcClient] client is closed! remote call failed!", emErr::ERR_CLIENT_CLOSE});
        }
    }

    m_reply_caller_map.clear();
}

bbt::core::errcode::ErrOpt RpcClient::OnReply(RemoteCallSeq seq, bbt::core::Buffer& buffer)
{
    // RpcReplyCallback onreply = nullptr;
    std::shared_ptr<detail::RemoteCaller> caller = nullptr;

    {
        std::lock_guard<std::mutex> lock(m_all_opt_mtx);
        auto it = m_reply_caller_map.find(seq);
        if (it == m_reply_caller_map.end())
        {
            return Errcode{BBT_RPC_ERR_PREFIX "[RpcClient] seq not found!", emErr::ERR_COMM};
        }

        caller = it->second;
        m_reply_caller_map.erase(it);
    }

    if (caller)
        caller->Reply(buffer, std::nullopt);

    return std::nullopt;
}

ErrOpt RpcClient::_DoReply(RemoteCallSeq seq, std::shared_ptr<detail::RemoteCaller> caller, const bbt::core::Buffer& buffer)
{
    {
        std::lock_guard<std::mutex> lock(m_all_opt_mtx);
        AssertWithInfo(m_reply_caller_map.find(seq) == m_reply_caller_map.end(), "uint64 not enough?");
        m_reply_caller_map[seq] = caller;
        m_timeout_queue.push(caller);
    }

    return m_tcp_client->Send(buffer);
}

void RpcClient::_Update()
{
    std::unique_lock<std::mutex> lock(m_all_opt_mtx);
    while (!m_timeout_queue.empty())
    {
        auto caller = m_timeout_queue.top();
        if (caller->GetTimeout() > clock::now())
            break;

        m_timeout_queue.pop();
        if (caller->IsReplyed())
            continue;
        Assert(m_reply_caller_map.erase(caller->GetSeq()));

        // 超时通知用户的时候不可以加锁
        lock.unlock();
        bbt::core::Buffer buffer{0};
        caller->Reply(buffer, Errcode{BBT_RPC_ERR_PREFIX "[RpcClient] reply is timeout!", emErr::ERR_CLIENT_TIMEOUT});
        lock.lock();
    }
}

bool RpcClient::IsConnected() const
{
    return m_tcp_client->IsConnected();
}

bbt::core::errcode::ErrOpt RpcClient::ReConnect()
{
    return m_tcp_client->ReConnect();
}


std::string RpcClient::DebugInfo()
{
    std::string info;
    info += "RpcClient Debug Info:\n";
    info += "  - Current Seq: " + std::to_string(m_current_seq) + "\n";
    info += "  - Reply Caller Map Size: " + std::to_string(m_reply_caller_map.size()) + "\n";
    info += "  - Timeout Queue Size: " + std::to_string(m_timeout_queue.size()) + "\n";
    return info;
}

} // namespace bbt::rpc