#pragma once
#include "Connection.h"
#include "../shared/all.h"
#include <bbt/templateutil/BaseType.hpp>
namespace yrpc::detail::net
{

class Acceptor: public bbt::templateutil::BaseType<Acceptor>
{
    typedef yrpc::coroutine::poller::Epoller    Epoller;
    typedef yrpc::detail::shared::errorcode     errorcode;
    typedef std::function<void(const yrpc::detail::shared::errorcode&, Connection::UQPtr)> OnAcceptCallback;
public:
    typedef std::function<Epoller*()>           LoadBalancer;
    /**
     * @brief Construct a new Acceptor object
     * 
     * @param loop 协程调度器
     * @param port 服务器端口号
     * @param socket_timeout_ms socket超时时间 
     * @param connect_timeout_ms 连接超时时间 
     */
    Acceptor(Epoller* loop, int port,int socket_timeout_ms = SOCKET_TIME_OUT_MS,int connect_timeout_ms = SOCKET_CONN_TIME_MS);
    ~Acceptor();
    /* 启动acceptor，需要先设置 loadbalancer 和 onaccept */
    int StartListen();
    /* 停止acceptor */
    void Close(){ m_closed.store(true); }
    /* 设置接收到连接的回调 */
    void SetOnAccept(const OnAcceptCallback& onconn, void* args = nullptr);
    /* 设置负载均衡的回调 */
    void SetLoadBalancer(const LoadBalancer& lber);
    /* 创建实例 */
    static UQPtr Create(Epoller* loop, int port, int socket_timeout_ms = 0, int connect_timeout_ms = 0);
protected:
    /* 在 loop 中开启监听 */
    void ListenInEvloop();
    void Init();
    void CreateListenSocket();
    void ReleaseListenSocket();
private:
    Epoller*        m_loop;
    LoadBalancer    m_lber;   
    Socket*         m_listenfd;
    int             m_port;
    int             m_fd;
    std::atomic_bool    m_closed;
    OnAcceptCallback    m_onaccept;
    void*               args_;

    int                 m_connect_timeout_ms;
    int                 m_socket_timeout_ms;

};  

}