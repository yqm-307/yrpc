#include "./Connection.h"

namespace yrpc::detail::net
{

Connection::Connection(yrpc::coroutine::poller::Epoller* scheduler,RoutineSocket* sockfd,const YAddress& cli)
    :m_socket(sockfd),
    m_schedule(scheduler),
    m_conn_status(connecting),
    m_cliaddr(cli)
{
    m_conn_status = connected;
    m_socket->socket_timeout_ = [this](RoutineSocket* socket){TimeOut(socket);};
    m_schedule->AddTask([this](void*ptr){recvhandler();},nullptr);      //  注册recv handler
    m_schedule->AddSocketTimer(m_socket);                               //  注册超时事件
    INFO("Connection::Connection() , info: connect success ! peer addr : %s",cli.GetIPPort().c_str());
}



Connection::~Connection()
{
    m_schedule->DestorySocket(m_socket);
}


size_t Connection::send(const char* data,size_t len)
{
    int n=0;
    if(m_conn_status == connected)
    {

        n = yrpc::socket::YRSend(*m_socket,data,len,0);    
        if(n<0)
        {
            int errorp = yrpc::util::tcp::GetSockErrCode(m_socket->sockfd_);
            if(errorp == EPIPE) //对端断开
                Close();
            ERROR("Connection::send() error , socket error %d",errorp);
        }
    }
    else if(m_conn_status == disconnect)
    {
        ERROR("Connection::send() error , conn_status is disconnect!");
        return -1;
    }
    return n;
}


size_t Connection::send(const Buffer& data)
{
    return send(data.peek(),data.ReadableBytes());
}


size_t Connection::recv(char* buffer,size_t buflen)
{
    int n = 0;
    if (m_conn_status == connected)
    {
        while (1)
        {
            n = yrpc::socket::YRRecv(*m_socket, buffer, buflen, 0);
            if (n == 0)
                Close();
            if (n < 0)
            {
                if (errno == EAGAIN)
                {
                    continue;
                }
                ERROR("Connection::recv() error , YRRecv error : %d ,errno is %s!", n, strerror(errno));
            }
            else
                break;
        }
    }
    else if (m_conn_status == disconnect)
    {
        ERROR("Connection::recv() error , conn_status is disconnect!");
        return -1;
    }
    return n;
}

size_t Connection::recv(Buffer& data)
{
    const int RECV_BUF_SIZE{4096};
    int n = 0;
    char buff[RECV_BUF_SIZE];
    
    if(0 > (n = recv(buff,RECV_BUF_SIZE)))    
        return -1;
    data.WriteString(buff,n);
    return n;
}



void Connection::Close()
{
    m_conn_status = disconnect;
    INFO("Connection::Close() : disconnect");
    yrpc::detail::shared::errorcode e;
    e.settype(yrpc::detail::shared::ERRTYPE_NETWORK);
    e.setinfo("Connection::Close() : disconnect");
    if(m_closecb != nullptr)
        m_closecb(e,shared_from_this());
}

void runhandle()
{
    
}


void Connection::recvhandler()
{

    yrpc::util::buffer::Buffer buffer;

    char* array = new char[4096];
    yrpc::detail::shared::errorcode e;
    e.settype(yrpc::detail::shared::ERRTYPE_NETWORK);

    while(m_conn_status)
    {
        memset(array,'\0',sizeof(buffer));
        int n = recv(array,sizeof(buffer));
        buffer.WriteString(array);        
        if(n > 0) 
        {
            e.setcode(yrpc::detail::shared::ERR_NETWORK_RECV_OK);
            
            DEBUG("Connection::recvhandler() debug , info: from internet recv %d bytes",n);
            if(m_onrecv)
            {
                m_onrecv(e,buffer);
            }
            else
                ERROR("Connection::recvhandler() error , info: recv handler is illegal!");
        }
    }

}

Connection::ConnectionPtr Connection::GetPtr()
{
    return this->shared_from_this();
}

void Connection::setOnRecvCallback(OnRecvHandle cb)
{
    m_onrecv = cb;
    update();
}

void Connection::setOnCloseCallback(ConnCloseHandle cb)
{
    m_closecb = cb;
}

void Connection::setOnTimeoutCallback(OnTimeoutCallback cb)
{
    m_timeoutcb = cb;
}


void Connection::update()
{
    m_schedule->AddTask([this](void *)
                       { recvhandler(); },
                       nullptr);
}

bool Connection::IsTimeOut()
{
    return m_socket->eventtype_ == yrpc::coroutine::poller::EpollREvent_Timeout;
}

bool Connection::IsClosed()
{
    return m_conn_status == disconnect;
}

const YAddress &Connection::GetPeerAddress() const
{
    return m_cliaddr;
}

std::string Connection::StrIPPort()
{
    return m_cliaddr.GetIPPort();
}

void Connection::TimeOut(RoutineSocket* socket)
{
    // 超时被调用,做超时处理
    // 网络层不做处理，交给上层处理超时的连接
    if (m_timeoutcb != nullptr)
        m_timeoutcb();  
}
}



