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
    while (!close_)
    {
        /*监听到新的socket连接，创建conn，并注册 协程任务*/
        struct sockaddr_in cliaddr;
        socklen_t len;
        int newfd = yrpc::socket::YRAccept(*listenfd_,reinterpret_cast<sockaddr*>(&cliaddr),&len);  //主动让出cpu，直到错误或者成功返回
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
            YAddress cli(inet_ntoa(cliaddr.sin_addr),ntohs(len));
            Connection::ConnectionPtr newconn = std::make_shared<Connection>(evloop,clisock,std::move(cli));
            //handle(newconn->GetPtr());  //不对，这里如果让出cpu， 程序就会阻塞到执行完，还是要runinloop 在epoll中执行
            this->onconnection_(e,newconn); // onconnection 不可以是长时间阻塞的调用
        }
    }
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