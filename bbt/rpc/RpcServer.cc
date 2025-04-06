#include <tuple>
#include <bbt/core/clock/Clock.hpp>
#include <bbt/rpc/RpcServer.hpp>

using namespace bbt::core::errcode;
using namespace bbt::network;
using namespace bbt::core;

namespace bbt::rpc
{

RpcServer::RpcServer(std::shared_ptr<EvThread> io_thread):
    m_tcp_server(std::make_shared<TcpServer>(io_thread))
{
}

RpcServer::~RpcServer()
{
    m_tcp_server = nullptr;
}

ErrOpt RpcServer::Init(const char* ip, int port, int connection_timeout)
{
    m_tcp_server->Init();

    m_tcp_server->SetOnSend([weak_this{weak_from_this()}](ConnId id, ErrOpt err, size_t len) {
        if (auto shared_this = weak_this.lock(); shared_this != nullptr)
            shared_this->OnSend(id, err, len);
    });
    m_tcp_server->SetOnRecv([weak_this{weak_from_this()}](ConnId id, const Buffer& buffer) {
        if (auto shared_this = weak_this.lock(); shared_this != nullptr)
            shared_this->OnRecv(id, buffer);
    });
    m_tcp_server->SetOnClose([weak_this{weak_from_this()}](ConnId id) {
        if (auto shared_this = weak_this.lock(); shared_this != nullptr)
            shared_this->OnClose(id);
    });
    m_tcp_server->SetOnTimeout([weak_this{weak_from_this()}](ConnId id) {
        if (auto shared_this = weak_this.lock(); shared_this != nullptr)
            shared_this->OnTimeout(id);
    });
    m_tcp_server->SetOnErr([weak_this{weak_from_this()}](const Errcode& err) {
        if (auto shared_this = weak_this.lock(); shared_this != nullptr)
            shared_this->OnError(err);
    });
    m_tcp_server->SetTimeout(connection_timeout);
    
    if (auto err = m_tcp_server->AsyncListen(IPAddress(ip, port), [weak_this{weak_from_this()}](auto connid){
        if (auto shared_this = weak_this.lock(); shared_this != nullptr)
            shared_this->OnAccept(connid);
    }); err.has_value())
    {
        return err;
    }

    return std::nullopt;
}

ErrOpt RpcServer::RegisterMethod(const char* method_name, const RpcMethod& method)
{
    std::lock_guard<std::mutex> lock(m_all_opt_mtx);
    auto hash = m_codec.GetMethodHash(method_name);
    auto it = m_method_map.find(hash);
    if (it != m_method_map.end())
    {
        return Errcode{"repeat regist method!", ERR_METHOD_ALREADY_REGISTERED};
    }

    m_method_map[hash] = method;
    return std::nullopt;
}

ErrOpt RpcServer::UnRegisterMethod(const char* method_name)
{
    std::lock_guard<std::mutex> lock(m_all_opt_mtx);
    auto it = m_method_map.find(m_codec.GetMethodHash(method_name));
    if (it == m_method_map.end())
    {
        return Errcode{"repeat regist method!", ERR_COMM};
    }

    m_method_map.erase(it);
    return std::nullopt;
}

bbt::core::errcode::ErrOpt RpcServer::DoReply(ConnId connid, RemoteCallSeq seq, const bbt::core::Buffer& results)
{
    auto buffer = bbt::core::Buffer{};
    detail::Helper::SerializeReq(buffer, 0, seq, results.Peek());

    return m_tcp_server->Send(connid, buffer);
}

std::string RpcServer::DebugInfo()
{
    std::lock_guard<std::mutex> lock(m_all_opt_mtx);
    std::string info;
    info += "RpcServer Debug Info:\n";
    info += "  Method Map Size: " + std::to_string(m_method_map.size()) + "\n";
    info += "  Buffer Map Size: " + std::to_string(m_buffer_mgr.Size()) + "\n";
    info += "  Buffer Total Size: " + std::to_string(m_buffer_mgr.GetTotalByte()) + "\n";
    return info;
}


void RpcServer::OnError(const bbt::core::errcode::Errcode& err)
{
    std::cerr << bbt::core::clock::getnow_str() << "[RpcServer::DefaultErr]" <<  " " << err.What() << std::endl;
}

void RpcServer::OnAccept(ConnId connid)
{
    std::lock_guard<std::mutex> lock(m_all_opt_mtx);

    m_buffer_mgr.AddBuffer(connid, Buffer());
}

void RpcServer::OnRecv(ConnId connid, const bbt::core::Buffer& buffer)
{
    std::vector<Buffer> protocols;

    if (buffer.Size() <= 0)
        return;

    {
        std::lock_guard<std::mutex> lock(m_all_opt_mtx);
    
        auto conn_buffer = m_buffer_mgr.GetBuffer(connid);
        if (!conn_buffer)
        {
            OnError(Errcode{"buffer not found!", ERR_COMM});
            return;
        }
    
        conn_buffer->WriteString(buffer.Peek(), buffer.Size());
        auto err = detail::Helper::ParseProtocolFromBuffer(*conn_buffer, protocols);
        if (err.has_value())
        {
            OnError(err.value());
            if (err->Type() == ERR_BAD_PROTOCOL_LENGTH_OVER_LIMIT)
                m_tcp_server->Close(connid);
            return;
        }
    }

    for (auto& protocol : protocols)
    {
        if (auto err = OnRemoteCall(connid, protocol); err.has_value())
        {
            OnError(err.value());
            return;
        }
    }
}

void RpcServer::OnClose(ConnId connid)
{
    std::lock_guard<std::mutex> lock(m_all_opt_mtx);
    m_buffer_mgr.RemoveBuffer(connid);
}

ErrOpt RpcServer::OnRemoteCall(ConnId connid, const bbt::core::Buffer& buffer)
{
    auto* head = (detail::ProtocolHead*)buffer.Peek();
    RpcMethod method = nullptr;
    RemoteCallSeq call_seq = head->call_seq;

    {
        std::lock_guard<std::mutex> lock(m_all_opt_mtx);
        auto it = m_method_map.find(head->method_hash);
        if (it == m_method_map.end())
        {
            return Errcode{"method not found!", ERR_SERVER_NO_METHOD};
        }

        method = it->second;
    }

    if (method)
    {
        bbt::core::Buffer body{buffer.Peek() + sizeof(detail::ProtocolHead), buffer.Size() - sizeof(detail::ProtocolHead)};
        method(shared_from_this(), connid, call_seq, body);
    }
    else
    {
        return Errcode{"method not found!", ERR_SERVER_NO_METHOD};
    }

    return std::nullopt;
}

}// namespace bbt::rpc