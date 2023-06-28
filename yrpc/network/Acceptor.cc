#include "Acceptor.h"
#include "../Util/all.h"
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <functional>

namespace yrpc::detail::net
{


Acceptor::Acceptor(int port,int socket_timeout_ms,int connect_timeout_ms)
    :listenfd_(nullptr),
    port_(port),
    close_(false),
    onconnection_(nullptr),
    connect_timeout_ms_(connect_timeout_ms),
    socket_timeout_ms_(socket_timeout_ms)
{
    assert(fd_>=0);
    Init();
}

Acceptor::~Acceptor()
{//释放套接字资源和Socket内存
    INFO("[YRPC][Acceptor::~Acceptor] Acceptor Destroy!");
    this->ReleaseListenSocket();
}

void Acceptor::Init()
{
    if( yrpc::socket::YRCreateListen(&fd_,port_) < 0 )
        ERROR("Acceptor::Acceptor() error , YRCreateListen call failed!");
    CreateListenSocket();
    //listenfd创建完成
    // scheduler_->AddTask([this](void*){StartListen();},nullptr);
}


int Acceptor::StartListen()
{
    if (close_ != false)
        return -1;
    if (onconnection_ == nullptr)
        return -2;
    y_scheduler->AddTask([this](void*){ListenInEvloop();},nullptr); // 注册监听任务
    INFO("[YRPC][Acceptor::listen] acceptor begin!");
    return 0;
}


// void Acceptor::ListenRunInLoop()
// {
//     while(!close_)
//         listen();
// }



void Acceptor::ListenInEvloop()
{
    // 创建一个协程任务,每50ms执行一次,处理连接事件
    y_scheduler->AddTimer([this](){
        /*监听到新的socket连接，创建conn，并注册 协程任务*/
        struct sockaddr_in localaddr, peeraddr;
        socklen_t len;
        int newfd = yrpc::socket::YRAccept(*listenfd_,reinterpret_cast<sockaddr*>(&localaddr),&len);  //主动让出cpu，直到错误或者成功返回
        //新连接到达
        if(newfd < 0)
        {
            int save_errno = errno;
            if( save_errno != EAGAIN ) {
                ERROR("[YRPC][Acceptor::ListenInEvloop] accept error, errno is %d!", save_errno);
            }
        }
        else
        {//创建conn
            errorcode e; e.settype(yrpc::detail::shared::YRPC_ERR_TYPE::ERRTYPE_NETWORK);
            if (newfd >= 0)
                e.setcode(yrpc::detail::shared::ERR_NETWORK::ERR_NETWORK_ACCEPT_OK);
            else
                e.setcode(yrpc::detail::shared::ERR_NETWORK::ERR_NETWORK_ACCEPT_FAIL);
            //创建socket

            auto evloop = ( lber_ == nullptr ) ? y_scheduler : lber_();  // 走不走负载均衡，不走就默认在当前线程进行IO
            Socket* clisock = evloop->CreateSocket(newfd,socket_timeout_ms_,connect_timeout_ms_);  //普通连接
            // 获取对端ip地址和端口
            len = sizeof(peeraddr);
            int succ = ::getpeername(newfd, reinterpret_cast<sockaddr*>(&peeraddr), &len);
            if (succ >= 0) {
                YAddress cli(inet_ntoa(peeraddr.sin_addr), htons(peeraddr.sin_port));
                Connection::ConnectionPtr newconn = std::make_shared<Connection>(evloop,clisock,std::move(cli));
                this->onconnection_(e,newconn); // onconnection 不可以是长时间阻塞的调用
            }
            else 
            {
                FATAL("[YRPC][Acceptor::ListenInEvloop] getpeername call failed!");
            }
        }

    },50,50,-1);
}


void Acceptor::CreateListenSocket()
{
    listenfd_ = y_scheduler->CreateSocket(fd_,-1,-1); //需要free
    if(listenfd_ == nullptr) 
        ERROR("Acceptor::CreateListenSocket() error , Epoller::CreateSocket error!");
}

void Acceptor::ReleaseListenSocket()
{
    if(listenfd_ == nullptr)
        ERROR("Acceptor::RelaseListenSocket() error , listenfd is null!");
    y_scheduler->DestorySocket(listenfd_);   //释放socket内存
    if(fd_ < 0 )
        ERROR("Acceptor::RelaseListenSocket() error , listen socket fd = %d error\n",fd_);
    ::close(fd_);
}



}