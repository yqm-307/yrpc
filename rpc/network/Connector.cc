#include "Connector.h"

using namespace yrpc::detail::net;


Connector::Connector(yrpc::coroutine::poller::Epoller* loop,YAddress servaddr,int socket_ms,int conn_ms)
    :scheduler_(loop),
    servfd_(nullptr),
    servaddr_(servaddr),
    fd_(::socket(AF_INET,SOCK_STREAM,0)),
    socket_timeout_ms_(socket_ms),
    connect_timeout_ms_(conn_ms)
{
    if(fd_ < 0 )
        FATAL("Connector::Connector() error , ::socket error!");
    assert(fd_>=0);
    CreateSocket();
    
}

Connector::~Connector()
{
    ReleaseSocket();
}


void Connector::CreateSocket()
{
    servfd_ = scheduler_->CreateSocket(fd_);
    assert(servfd_ != nullptr);
}

void Connector::ReleaseSocket()
{
    assert(fd_>=0);
    scheduler_->DestorySocket(servfd_); //释放内存
    ::close(fd_);   //关闭套接字描述符
}



void Connector::connect()
{
    scheduler_->AddTask([this](void*){onConnect();},nullptr);    // 注册协程
}

void Connector::onConnect()
{
    int n = yrpc::socket::YRConnect(*servfd_,servaddr_.getsockaddr(),servaddr_.getsocklen());
    int connerror=0;
    socklen_t len = sizeof(connerror);
    getsockopt(servfd_->sockfd_,SOL_SOCKET,SO_ERROR,&connerror,&len);
    if(connerror == ECONNREFUSED)
    {
        ERROR("Connector::connect() error , YRConnect error : connect refused %d",connerror);
        onclosed_();
        return;
    }
    if(n<0)
    {
        ERROR("Connector::connect() error , YRConnect error : retvalue %d",n);
    }
    else
    {//建立新连接
        ConnectionPtr conn = std::make_shared<Connection>(scheduler_,servfd_,servaddr_);
        if(onconnect_)
        {
            INFO("Connector::connect() ,connet success peer %s",conn->StrIPPort().c_str());
            onconnect_(conn,args_);//直接执行没问题，连接要么成功要么失败，和Acceptor不一样，不需要循环处理，阻塞就阻塞。
        }
        else
            INFO("Connector no connect handle!");
    }
}


