#pragma once
#include <bbt/network/TcpClient.hpp>

#include <bbt/rpc/detail/Define.hpp>
#include <bbt/rpc/detail/RpcCodec.hpp>
#include <bbt/rpc/detail/Protocol.hpp>
#include <bbt/rpc/detail/RemoteCaller.hpp>


namespace bbt::rpc
{

class RpcClient:
    public std::enable_shared_from_this<RpcClient>,
    private boost::noncopyable
{
public:
    RpcClient(std::shared_ptr<bbt::network::EvThread> io_thread);
    virtual~RpcClient();

    bbt::core::errcode::ErrOpt  Init(const char* ip, int port, int connect_timeout = 10000, int connection_timeout = 0,
                                    const std::function<void(std::shared_ptr<RpcClient>)>& on_connect_callback = nullptr);

    template<typename... Args>
    bbt::core::errcode::ErrOpt  RemoteCall(const char* method_name, int timeout, const RpcReplyCallback& callback, Args&&... args);

    std::string                 DebugInfo();

    bool                        IsConnected() const;
    bbt::core::errcode::ErrOpt  ReConnect();

protected:
    virtual void                OnTimeout(ConnId id) {}
    virtual void                OnSend(ConnId id, bbt::core::errcode::ErrOpt err, size_t len) {}
    virtual void                OnError(const bbt::core::errcode::Errcode& err);
private:
    void                        OnRecv(ConnId id, const bbt::core::Buffer& buffer);
    void                        OnConnect(ConnId id, bbt::core::errcode::ErrOpt err);
    void                        OnClose(ConnId id);
    void                        FailedAll();
    bbt::core::errcode::ErrOpt  OnReply(RemoteCallSeq seq, const bbt::core::Buffer& buffer);
    bbt::core::errcode::ErrOpt  _DoReply(RemoteCallSeq seq, std::shared_ptr<detail::RemoteCaller> caller, const bbt::core::Buffer& buffer);
    void                        _Update();
private:
    std::shared_ptr<bbt::network::TcpClient>        m_tcp_client{nullptr};

    std::atomic<RemoteCallSeq>      m_current_seq{0};

    std::mutex                      m_all_opt_mtx;
    std::unordered_map<RemoteCallSeq, std::shared_ptr<detail::RemoteCaller>>
                                    m_reply_caller_map;
    std::priority_queue<
        std::shared_ptr<detail::RemoteCaller>,
        std::vector<std::shared_ptr<detail::RemoteCaller>>,
        detail::RemoteCallerComp>   m_timeout_queue;
    bbt::core::Buffer               m_recv_buffer;
    std::function<void(std::shared_ptr<RpcClient>)> 
                                    m_on_connect_callback{nullptr};
    std::shared_ptr<bbt::pollevent::Event> 
                                    m_update_event{nullptr};
};

template<typename... Args>
bbt::core::errcode::ErrOpt RpcClient::RemoteCall(const char* method_name, int timeout, const RpcReplyCallback& callback, Args&&... args)
{
    static detail::RpcCodec codec;

    if (!m_tcp_client->IsConnected())
        return bbt::core::errcode::Errcode{"client not connected!", emErr::ERR_CLIENT_CLOSE};

    bbt::core::Buffer buffer;
    auto seq = ++m_current_seq;

    std::shared_ptr<detail::RemoteCaller> caller = std::make_shared<detail::RemoteCaller>(timeout, seq, callback);
    
    auto hash = codec.GetMethodHash(method_name);
    detail::Helper::SerializeReq(buffer, hash, seq, std::forward<Args>(args)...);

    return _DoReply(seq, caller, buffer);
}



} // namespace bbt::rpc