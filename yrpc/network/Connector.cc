#include "Connector.h"

using namespace yrpc::detail::net;


Connector::Connector(yrpc::coroutine::poller::Epoller* loop)
    :scheduler_(loop)
{
    YAssert(loop != nullptr, "[Connector::Connector] Epoller is nullptr!");
}

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
    if(onconnect_ != nullptr) 
    {
        if(connerror == ECONNREFUSED)
        {// 连接被拒绝
            e.setcode(yrpc::detail::shared::ERR_NETWORK_ECONNREFUSED);
            e.setinfo("connect refused %d",connerror);
            onconnect_(e, servaddr_, nullptr);
            return;
        }
        if(n<0)
        {// 其他错误（对端不存在等）
            e.setcode(yrpc::detail::shared::ERR_NETWORK_CONN_OTHRE_ERR);
            e.setinfo("connect failed! errno is %d", errno);
            onconnect_(e, servaddr_, nullptr);
            ERROR("[YRPC][Connector::onConnect] connect error, return is  %d",n);
            return;
        }
        else
        {//成功建立新连接
            ConnectionPtr conn = std::make_shared<Connection>(scheduler_,servfd_,servaddr_);
            e.setcode(yrpc::detail::shared::ERR_NETWORK_CONN_OK);
            e.setinfo("connet success peer %s",conn->StrIPPort().c_str());
            onconnect_(e, servaddr_, conn);//直接执行没问题，连接要么成功要么失败，和Acceptor不一样，不需要循环处理，阻塞就阻塞。
        }
    }
    else
    {
        INFO("[YRPC][Connector::onConnect] onconnect_ is nullptr!");
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

void Connector::SyncConnect(Socket* socket, YAddress servaddr)
{
    
}