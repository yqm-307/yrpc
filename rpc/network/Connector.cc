#include "Connector.h"

using namespace yrpc::detail::net;


Connector::Connector(yrpc::coroutine::poller::Epoller* loop)
    :scheduler_(loop)
{}

Connector::~Connector()
{}







void Connector::onConnect(Socket* servfd_,const YAddress& servaddr_,const OnConnectHandle& onconnect_)
{
    int n = yrpc::socket::YRConnect(*servfd_,servaddr_.getsockaddr(),servaddr_.getsocklen());
    int connerror=0;
    socklen_t len = sizeof(connerror);
    getsockopt(servfd_->sockfd_,SOL_SOCKET,SO_ERROR,&connerror,&len);

    yrpc::detail::shared::errorcode e;
    e.settype(yrpc::detail::shared::ERRTYPE_NETWORK);
    if(connerror == ECONNREFUSED)
    {// 连接被拒绝
        e.setcode(yrpc::detail::shared::ERR_NETWORK_ECONNREFUSED);
        e.setinfo("Connector::connect() error , YRConnect error : connect refused %d",connerror);
        onconnect_(e,nullptr);
        return;
    }
    if(n<0)
    {

        ERROR("Connector::connect() error , YRConnect error : retvalue %d",n);
    }
    else
    {//成功建立新连接
        ConnectionPtr conn = std::make_shared<Connection>(scheduler_,servfd_,servaddr_);
        if(onconnect_ != nullptr)
        {
            e.setcode(yrpc::detail::shared::ERR_NETWORK_CONN_OK);
            e.setinfo("Connector::connect() ,connet success peer %s",conn->StrIPPort().c_str());
            onconnect_(e,conn);//直接执行没问题，连接要么成功要么失败，和Acceptor不一样，不需要循环处理，阻塞就阻塞。
        }
        else
            INFO("Connector no connect handle!");
    }
}



Socket* Connector::CreateSocket()
{
    Socket* socket = new Socket();
    socket->sockfd_ = ::socket(AF_INET,SOCK_STREAM,0);
    return socket;
}

void Connector::DestorySocket(Socket*socket)
{
    ::close(socket->sockfd_);
    delete socket;
}