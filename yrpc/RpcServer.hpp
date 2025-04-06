#pragma once
#include <yrpc/detail/RpcCodec.hpp>
#include <yrpc/detail/Protocol.hpp>
#include <yrpc/detail/BufferMgr.hpp>

namespace yrpc
{

class RpcServer:
    public std::enable_shared_from_this<RpcServer>,
    private boost::noncopyable
{
public:
    /**
     * @brief 构造一个监听ip和端口的TcpServer，并运行在io_thread上
     * 
     * @param io_thread 
     * @param ip 
     * @param port 
     */
    RpcServer(std::shared_ptr<bbt::network::EvThread> io_thread);
    virtual ~RpcServer();

    bbt::core::errcode::ErrOpt Init(const char* ip, int port, int connection_timeout = 10000);

    bbt::core::errcode::ErrOpt RegisterMethod(const char* method_name, const RpcMethod& method);
    bbt::core::errcode::ErrOpt UnRegisterMethod(const char* method_name);

    template<typename... Args>
    bbt::core::errcode::ErrOpt DoReply(ConnId connid, RemoteCallSeq seq, Args&&... args);

    bbt::core::errcode::ErrOpt DoReply(ConnId connid, RemoteCallSeq seq, const bbt::core::Buffer& results);

    std::string DebugInfo();
protected:
    virtual void OnError(const bbt::core::errcode::Errcode& err);
    virtual void OnTimeout(ConnId connid) {}
    virtual void OnSend(ConnId connid, bbt::core::errcode::ErrOpt err, size_t send_len) {}

private:
    void OnAccept(ConnId connid);
    void OnRecv(ConnId connid, const bbt::core::Buffer& buffer);
    void OnClose(ConnId connid);

    bbt::core::errcode::ErrOpt OnRemoteCall(ConnId connid, const bbt::core::Buffer& buffer);
private:
    detail::RpcCodec m_codec;

    std::shared_ptr<bbt::network::TcpServer> m_tcp_server{nullptr};

    std::mutex m_all_opt_mtx;

    std::unordered_map<RpcMethodHash, RpcMethod> m_method_map;
    detail::BufferMgr m_buffer_mgr;
};


template<typename... Args>
bbt::core::errcode::ErrOpt RpcServer::DoReply(ConnId connid, RemoteCallSeq seq, Args&&... args)
{
    auto buffer = bbt::core::Buffer{};
    detail::Helper::SerializeReq(buffer, 0, seq, std::forward<Args>(args)...);
    return m_tcp_server->Send(connid, buffer);
}

} // namespace yrpc