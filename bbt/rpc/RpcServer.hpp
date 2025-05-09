#pragma once
#include <bbt/rpc/detail/Protocol.hpp>
#include <bbt/rpc/detail/BufferMgr.hpp>

namespace bbt::rpc
{

/**
 * @brief 运行在一个evthread上的RpcServer
 * 
 * 1、提供动态注册方法和注销方法；
 * 2、基于tcp长连接；
 * 3、独立线程运行，实现异步发送；
 */
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

    /**
     * @brief 初始化RpcServer配置
     * 
     * @param ip 
     * @param port 
     * @param connection_timeout 
     * @return bbt::core::errcode::ErrOpt 
     */
    bbt::core::errcode::ErrOpt Init(const char* ip, int port, int connection_timeout = 10000);

    /**
     * @brief 注册一个方法，线程安全的
     * 
     * @param method_name 
     * @param method 
     * @return bbt::core::errcode::ErrOpt 
     */
    bbt::core::errcode::ErrOpt RegisterMethod(const char* method_name, const RpcMethod& method);

    /**
     * @brief 反注册一个方法，线程安全的
     * 
     * @param method_name 
     * @return bbt::core::errcode::ErrOpt 
     */
    bbt::core::errcode::ErrOpt UnRegisterMethod(const char* method_name);

    template<typename Tuple>
    bbt::core::errcode::ErrOpt DoReply(ConnId connid, RemoteCallSeq seq, Tuple&& args);

    /**
     * @brief 向指定连接发送一个Rpc响应
     * 与上一个函数的区别在于，发送的数据是已经序列化好的
     * 
     * @param connid 
     * @param seq 
     * @param results 
     * @return bbt::core::errcode::ErrOpt 
     */
    bbt::core::errcode::ErrOpt DoReply(ConnId connid, RemoteCallSeq seq, const bbt::core::Buffer& results);

    std::string DebugInfo();
protected:

    /**
     * @brief 由子类重写，错误处理函数
     * 
     * @param err 
     */
    virtual void OnError(const bbt::core::errcode::Errcode& err);

    /**
     * @brief 连接超时处理函数
     * 
     * @param connid 
     */
    virtual void OnTimeout(ConnId connid) {}

    /**
     * @brief 发送完成处理函数
     * 
     * @param connid 
     * @param err 
     * @param send_len 
     */
    virtual void OnSend(ConnId connid, bbt::core::errcode::ErrOpt err, size_t send_len) {}

    virtual void OnAccept(ConnId connid, const bbt::network::IPAddress& addr) {}

private:
    void _OnAccept(ConnId connid);
    void OnRecv(ConnId connid, const bbt::core::Buffer& buffer);
    void OnClose(ConnId connid);

    bbt::core::errcode::ErrOpt OnRemoteCall(ConnId connid, const bbt::core::Buffer& buffer);
private:
    std::shared_ptr<bbt::network::TcpServer> m_tcp_server{nullptr};

    std::mutex m_all_opt_mtx;

    std::unordered_map<RpcMethodHash, RpcMethod> m_method_map;
    detail::BufferMgr m_buffer_mgr;
};

template<typename Tuple>
bbt::core::errcode::ErrOpt RpcServer::DoReply(ConnId connid, RemoteCallSeq seq, Tuple&& args)
{
    auto buffer = bbt::core::Buffer{};
    detail::Helper::SerializeReqWithTuple(buffer, 0, seq, std::forward<Tuple>(args));
    return m_tcp_server->Send(connid, buffer);
}
} // namespace bbt::rpc