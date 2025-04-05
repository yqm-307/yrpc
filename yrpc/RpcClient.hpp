#pragma once
#include <yrpc/detail/Define.hpp>
#include <yrpc/detail/RpcCodec.hpp>
#include <yrpc/detail/Protocol.hpp>
#include <bbt/network/TcpClient.hpp>


namespace yrpc
{

class RpcClient:
    public std::enable_shared_from_this<RpcClient>,
    private boost::noncopyable
{
public:
    RpcClient(std::shared_ptr<bbt::network::EvThread> io_thread);
    ~RpcClient();

    bbt::core::errcode::ErrOpt  Init(const char* ip, int port, int connect_timeout = 10000, int connection_timeout = 0,
                                    const std::function<void(std::shared_ptr<RpcClient>)>& on_connect_callback = nullptr);

    template<typename... Args>
    bbt::core::errcode::ErrOpt  RemoteCall(const char* method_name, const RpcReplyCallback& callback, Args&&... args);


protected:
    virtual void                OnTimeout(ConnId id) {}
    virtual void                OnSend(ConnId id, bbt::core::errcode::ErrOpt err, size_t len) {}
    virtual void                OnError(const bbt::core::errcode::Errcode& err) {}
private:
    void                        OnRecv(ConnId id, const bbt::core::Buffer& buffer);
    void                        OnConnect(ConnId id, bbt::core::errcode::ErrOpt err);
    void                        OnClose(ConnId id);
    void                        FailedAll();
    bbt::core::errcode::ErrOpt  OnReply(RemoteCallSeq seq, const bbt::core::Buffer& buffer);
private:
    std::shared_ptr<bbt::network::TcpClient>        m_tcp_client{nullptr};

    std::atomic<RemoteCallSeq>      m_current_seq{0};

    std::mutex                      m_all_opt_mtx;
    std::unordered_map<RemoteCallSeq, RpcReplyCallback> m_reply_callback_map;
    bbt::core::Buffer               m_recv_buffer;
    std::function<void(std::shared_ptr<RpcClient>)> m_on_connect_callback{nullptr};
};

template<typename... Args>
bbt::core::errcode::ErrOpt RpcClient::RemoteCall(const char* method_name, const RpcReplyCallback& callback, Args&&... args)
{
    static detail::RpcCodec codec;
    bbt::core::Buffer buffer;
    
    auto hash = codec.GetMethodHash(method_name);
    auto seq = ++m_current_seq;
    detail::Helper::SerializeReq(buffer, hash, seq, std::forward<Args>(args)...);

    {
        std::lock_guard<std::mutex> lock(m_all_opt_mtx);
        AssertWithInfo(m_reply_callback_map.find(seq) == m_reply_callback_map.end(), "uint64 not enough?");
        m_reply_callback_map[seq] = std::move(callback);
    }

    return m_tcp_client->Send(buffer);
}

} // namespace yrpc