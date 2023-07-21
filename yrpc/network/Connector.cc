#include "Connector.h"

using namespace yrpc::detail::net;


Connector::Connector(yrpc::coroutine::poller::Epoller* loop)
    :scheduler_(loop)
{
    YAssert(loop != nullptr, "[Connector::Connector] Epoller is nullptr!");
}

Connector::~Connector()
{}

void Connector::Connect(const YAddress& peer_addr)
{
    /* 对于socket fd ，其实这里交给了connection管理，会在connection析构close，如果有可能的话想要做的更加现代 */
    int sockfd = ::socket(AF_INET, SOCK_STREAM, 0);
    /* 取到对于线程的调度器，将connection的后续active都放在某个固定线程执行，经典reactor做法 */
    auto poll = m_lber();
    assert(poll != nullptr);
    Socket::RawPtr tmp_socket = yrpc::socket::CreateSocket(sockfd, poll, poll->GetPollFd());

    int n = yrpc::socket::YRConnect(*tmp_socket ,peer_addr.getsockaddr(), peer_addr.getsocklen());
    int connerror = errno;
    socklen_t len = sizeof(connerror);
    connerror = yrpc::util::tcp::GetSockErrCode(tmp_socket->sockfd_);

    yrpc::detail::shared::errorcode e;
    e.settype(yrpc::detail::shared::ERRTYPE_NETWORK);
    if(m_onconn != nullptr) 
    {
        assert(m_lber != nullptr);
        auto conn_in_loop = m_lber(); 
        Connection::UQPtr conn = Connection::Create(conn_in_loop, tmp_socket, peer_addr);
        if(connerror == ECONNREFUSED)
        {// 连接被拒绝
            e.setcode(yrpc::detail::shared::ERR_NETWORK_ECONNREFUSED);
            e.setinfo("connect refused %d",connerror);
            ERROR("[YRPC][Connector::Connect][%d] connect err! errno: %d",y_scheduler_id, connerror);
            m_onconn(e, std::move(conn), peer_addr);
            return;
        }
        if(n<0)
        {// 其他错误（对端不存在等）
            e.setcode(yrpc::detail::shared::ERR_NETWORK_CONN_OTHRE_ERR);
            e.setinfo("connect failed! errno is %d", errno);
            ERROR("[YRPC][Connector::Connect][%d] connect err! errno: %d", y_scheduler_id, connerror);
            m_onconn(e, std::move(conn), peer_addr);
            return;
        }
        else
        {//成功建立新连接
            e.setcode(yrpc::detail::shared::ERR_NETWORK_CONN_OK);
            e.setinfo("connet success peer %s",conn->StrIPPort().c_str());
            DEBUG("[YRPC][Connector::Connect][%d] connect succ", y_scheduler_id);
            m_onconn(e, std::move(conn), peer_addr);//直接执行没问题，连接要么成功要么失败，和Acceptor不一样，不需要循环处理，阻塞就阻塞。
        }
    }
    else
    {
        ERROR("[YRPC][Connector::onConnect] onconnect_ is nullptr!");
    }

}

void Connector::AsyncConnect(YAddress servaddr)
{   
    scheduler_->AddTask([this,servaddr](void*){
        this->Connect(servaddr);
    });
}

void Connector::SetLoadBalancer(const LoadBalancer& lber)
{ 
    m_lber = lber; 
}

void Connector::SetOnConnectCallback(const OnConnectCallback& cb)
{ 
    m_onconn = cb; 
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

Connector::UQPtr Connector::Create(yrpc::coroutine::poller::Epoller* loop)
{
    return std::make_unique<Connector>(loop);
}