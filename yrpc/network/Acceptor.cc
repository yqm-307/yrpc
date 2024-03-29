#include "Acceptor.h"
#include "../Util/all.h"
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <functional>

namespace yrpc::detail::net
{


Acceptor::Acceptor(Epoller* loop, int port,int socket_timeout_ms,int connect_timeout_ms)
    :m_loop(loop),
    m_listenfd(nullptr),
    m_port(port),
    m_closed(false),
    m_onaccept(nullptr),
    m_connect_timeout_ms(connect_timeout_ms),
    m_socket_timeout_ms(socket_timeout_ms)
{
    Init();
}

Acceptor::~Acceptor()
{//释放套接字资源和Socket内存
    INFO("[YRPC][Acceptor::~Acceptor] Acceptor Destroy!");
    this->ReleaseListenSocket();
    
}

void Acceptor::Init()
{
    if( yrpc::socket::YRCreateListen(&m_fd, m_port) < 0 )
        ERROR("Acceptor::Acceptor() error , YRCreateListen call failed!");
    CreateListenSocket();
}


int Acceptor::StartListen()
{
    if (m_closed != false)
        return -1;
    if (m_onaccept == nullptr)
        return -2;
    m_loop->AddTask([this](void*){ListenInEvloop();},nullptr); // 注册监听任务
    INFO("[YRPC][Acceptor::StartListen][%d] acceptor begin!", y_scheduler_id);
    return 0;
}

void Acceptor::ListenInEvloop()
{
    // 创建一个协程任务,每50ms执行一次,处理连接事件
    /*监听到新的socket连接，创建conn，并注册 协程任务*/
    struct sockaddr_in localaddr, peeraddr;
    memset(&localaddr, '\0', sizeof(localaddr));
    memset(&peeraddr, '\0', sizeof(peeraddr));
    socklen_t len = sizeof(localaddr);
    int newfd = yrpc::socket::YRAccept(*m_listenfd, reinterpret_cast<sockaddr*>(&localaddr), &len);  //主动让出cpu，直到错误或者成功返回
    //新连接到达
    if(newfd < 0)
    {
        int save_errno = errno;
        if( save_errno != EAGAIN ) {
            ERROR("[YRPC][Acceptor::ListenInEvloop][%d] accept error, errno is %d!", y_scheduler_id, save_errno);
        }
    }
    else
    {//创建conn
        errorcode e; e.settype(yrpc::detail::shared::YRPC_ERR_TYPE::ERRTYPE_NETWORK);
        if (newfd >= 0)
            e.setcode(yrpc::detail::shared::ERR_NETWORK::ERR_NETWORK_ACCEPT_OK);
        else
            e.setcode(yrpc::detail::shared::ERR_NETWORK::ERR_NETWORK_ACCEPT_FAIL);

        auto evloop = m_lber();
        assert(evloop != nullptr);
        Socket::RawPtr clisock = yrpc::socket::CreateSocket(newfd, evloop, evloop->GetPollFd(), m_socket_timeout_ms, m_connect_timeout_ms);  //普通连接
        // 获取对端ip地址和端口
        len = sizeof(peeraddr);
        int succ = ::getpeername(newfd, reinterpret_cast<sockaddr*>(&peeraddr), &len);
        if (succ >= 0) {
            YAddress cli(inet_ntoa(peeraddr.sin_addr), htons(peeraddr.sin_port));
            Connection::UQPtr newconn = Connection::Create(evloop, clisock, std::move(cli));
            DEBUG("[YRPC][Acceptor::ListenInEvloop][%d] accept success! peer{%s}", y_scheduler_id, newconn->GetPeerAddress().GetIPPort().c_str());
            this->m_onaccept(e, std::move(newconn)); // onconnection 不可以是长时间阻塞的调用
        }
        else 
        {
            FATAL("[YRPC][Acceptor::ListenInEvloop][%d] getpeername call failed!", y_scheduler_id);
        }
    }
    y_scheduler->AddTask([this](void *){
        ListenInEvloop();
    });
}


void Acceptor::CreateListenSocket()
{
    assert(m_loop != nullptr);
    m_listenfd = yrpc::socket::CreateSocket(m_fd, m_loop, m_loop->GetPollFd(), -1, -1); //需要free
    if(m_listenfd == nullptr) 
        ERROR("Acceptor::CreateListenSocket() error , Epoller::CreateSocket error!");
}

void Acceptor::ReleaseListenSocket()
{
    if(m_listenfd == nullptr)
        ERROR("Acceptor::RelaseListenSocket() error , listenfd is null!");
    yrpc::socket::DestorySocket(m_listenfd);   //释放socket内存
    if(m_fd < 0 )
        ERROR("Acceptor::RelaseListenSocket() error , listen socket fd = %d error\n",m_fd);
    ::close(m_fd);
}

Acceptor::UQPtr Acceptor::Create(Epoller* loop, int port, int socket_timeout_ms, int connect_timeout_ms)
{
    if( socket_timeout_ms > 0 && connect_timeout_ms > 0 )
        return std::make_unique<Acceptor>(loop, port, socket_timeout_ms, connect_timeout_ms);
    else if( socket_timeout_ms )
        return std::make_unique<Acceptor>(loop, port, socket_timeout_ms);
    else
        return std::make_unique<Acceptor>(loop, port);
}

void Acceptor::SetOnAccept(const OnAcceptCallback& onconn, void* args)
{ 
    m_onaccept = onconn;
    args_ = args;
}

void Acceptor::SetLoadBalancer(const LoadBalancer& lber)
{ 
    m_lber = lber; 
}

}