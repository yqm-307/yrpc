#include "Acceptor.h"
#include "../Util/all.h"
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <functional>

namespace yrpc::detail::ynet
{


Acceptor::Acceptor(yrpc::coroutine::poller::Epoller* loop,int port,int socket_timeout_ms,int connect_timeout_ms)
    :scheduler_(loop),
    close_(false),
    listenfd_(nullptr),
    connect_timeout_ms_(connect_timeout_ms),
    socket_timeout_ms_(socket_timeout_ms)
{
    if( yrpc::socket::YRCreateListen(&fd_,port) < 0 )
        ERROR("Acceptor::Acceptor() error , YRCreateListen call failed!");
    assert(loop!=nullptr);
    assert(fd_>=0);
    this->CreateListenSocket();
    //listenfd创建完成
    scheduler_->AddTask([this](void*){listen();},nullptr);
}


Acceptor::~Acceptor()
{//释放套接字资源和Socket内存
    this->ReleaseListenSocket();
}


void Acceptor::listen()
{
    while(!close_)
    {
        /*监听到新的socket连接，创建conn，并注册 协程任务*/
        struct sockaddr_in cliaddr;
        socklen_t len;
        int newfd = yrpc::socket::YRAccept(*listenfd_,reinterpret_cast<sockaddr*>(&cliaddr),&len);  //主动让出cpu，直到错误或者成功返回
        //新连接到达
        if(newfd < 0)
            ERROR("Acceptor::listen() error , YRAcceptor error!");
        else
        {//创建conn
            //创建socket
            RoutineSocket* clisock = scheduler_->CreateSocket(newfd,socket_timeout_ms_,connect_timeout_ms_);  //普通连接
            YAddress cli(inet_ntoa(cliaddr.sin_addr),ntohs(len));
            Connection::ConnectionPtr newconn = std::make_shared<Connection>(scheduler_,clisock,std::move(cli));
            //handle(newconn->GetPtr());  //不对，这里如果让出cpu， 程序就会阻塞到执行完，还是要runinloop 在epoll中执行
            scheduler_->AddTask(std::bind(onconnection_,newconn,_1),args_);
        }
        //todo 注册协程任务
        //新连接到达时的处理协程
    }
    //退出，关闭
    INFO("Acceptor::listen() , acceptor closed!");
}

void Acceptor::CreateListenSocket()
{
    listenfd_ = scheduler_->CreateSocket(fd_,-1,-1); //需要free
    if(listenfd_ == nullptr) 
        ERROR("Acceptor::CreateListenSocket() error , Epoller::CreateSocket error!");
}

void Acceptor::ReleaseListenSocket()
{
    if(listenfd_ == nullptr)
        ERROR("Acceptor::RelaseListenSocket() error , listenfd is null!");
    scheduler_->DestorySocket(listenfd_);   //释放socket内存
    if(fd_ < 0 )
        ERROR("Acceptor::RelaseListenSocket() error , listen socket fd = %d error\n",fd_);
    ::close(fd_);
}


}