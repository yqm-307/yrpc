#include "./Connection.h"

namespace yrpc::detail::net
{

Connection::Connection(yrpc::coroutine::poller::Epoller* scheduler,Socket* sockfd,const YAddress& cli)
    :m_socket(sockfd),
    m_schedule(scheduler),
    m_conn_status(connecting),
    m_cliaddr(cli),
    m_input_buffer(4096),
    m_output_buffer(4096)
{
    m_conn_status = connected;
    m_socket->socket_timeout_ = [this](Socket* socket){TimeOut(socket);};
    m_socket->scheduler = scheduler;
    // m_socket->socket_timeout_ms_ = 3000;
    // m_schedule->AddTask([this](void*ptr){RunInEvloop();},nullptr);      //  注册recv handler
    m_schedule->AddSocketTimer(m_socket);                               //  注册超时事件
    DEBUG("[YRPC][Connection::Connection][%d] info: connect success ! peer addr : %s", y_scheduler_id, cli.GetIPPort().c_str());
}



Connection::~Connection()
{
    if( m_conn_status != disconnect )
        Close();
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
            ERROR("[YRPC][Connection::send][%d] send errno: %d, %s", y_scheduler_id, errorp, strerror(errorp));
        }
    }
    else if(m_conn_status == disconnect)
    {
        ERROR("[YRPC][Connection::send][%d] conn_status is disconnect!", y_scheduler_id);
        return -1;
    }
    return n;
}


size_t Connection::send(const Buffer& data)
{
    return send(data.Peek(),data.ReadableBytes());
}


size_t Connection::recv(char* buffer,size_t buflen)
{
    int n = 0;
    if (m_conn_status == connected)
    {
        int debug_cycle_count = 0;
        while (1)
        {
            debug_cycle_count = debug_cycle_count > 0 ? (debug_cycle_count + 1) : debug_cycle_count;
            if(debug_cycle_count > 10) {
                ERROR("[YRPC][Connection::recv] cycle too much!");
                debug_cycle_count = -1;
            }
            n = yrpc::socket::YRRecv(*m_socket, buffer, buflen, 0);
            if (n == 0)
                Close();
            if (n < 0)
            {
                if (errno == EAGAIN)
                {
                    continue;
                }
                ERROR("[YRPC][Connection::recv][%d] YRRecv error : %d ,errno is %s!", y_scheduler_id, errno, strerror(errno));
            }
            else
                break;
        }
    }
    else if (m_conn_status == disconnect)
    {
        ERROR("[YRPC][Connection::recv][%d] conn_status is disconnect!", y_scheduler_id);
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

bool Connection::is_connected()
{
    return m_conn_status == connected;
}

void Connection::Close()
{
    m_conn_status = disconnect;
    // DEBUG("[YRPC][Connection::Close][%d] disconnect", y_scheduler_id);
    yrpc::detail::shared::errorcode e;
    e.settype(yrpc::detail::shared::ERRTYPE_NETWORK);
    e.setinfo("disconnect");
    this->m_socket->scheduler->CancelSocketTimer(this->m_socket);
    ::close(m_socket->sockfd_);
    INFO("[YRPC][Connection::Close][%d] close connect!", y_scheduler_id);
    if(m_closecb != nullptr)
        m_closecb(e, shared_from_this());
    else
        WARN("[YRPC][Connection::Close][%d] close func is nullptr", y_scheduler_id);
    yrpc::socket::DestorySocket(m_socket);
}

void Connection::RecvFunc()
{
    yrpc::detail::shared::errorcode e;
    e.settype(yrpc::detail::shared::ERRTYPE_NETWORK);
    assert(CheckScheduler());
    
    if( m_conn_status == connected )
    {
        m_input_buffer.InitAll();
        int n = recv(m_input_buffer.Peek(), m_init_buffer_size);
        m_input_buffer.WriteNull(n);
        if(n > 0) 
        {
            e.setcode(yrpc::detail::shared::ERR_NETWORK_RECV_OK);
            if(m_onrecv)
            {
                m_onrecv(e,m_input_buffer);
            }
            else
                ERROR("[YRPC][Connection::RecvFunc][%d] info: recv handler is illegal!", y_scheduler_id);
        }
        y_scheduler->AddTask([=](void* ){ RecvFunc(); });
    }
}

bool Connection::CheckScheduler()
{
    return y_scheduler_id == m_schedule->GetID();
}

void Connection::setOnRecvCallback(OnRecvHandle cb)
{
    m_onrecv = cb;
    // update();
}

void Connection::setOnCloseCallback(ConnCloseHandle cb)
{
    m_closecb = cb;
}

void Connection::setOnTimeoutCallback(OnTimeoutCallback cb)
{
    m_timeoutcb = cb;
}


void Connection::StartRecvFunc()
{
    m_schedule->AddTask([this](void *){ RecvFunc(); });
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

void Connection::TimeOut(Socket* socket)
{
    // 超时被调用,做超时处理
    // 网络层不做处理，交给上层处理超时的连接
    if ( m_timeoutcb != nullptr )
        m_timeoutcb(socket);
}

ConnectionPtr Connection::Create(yrpc::coroutine::poller::Epoller* scheduler,Socket* sockfd,const YAddress& cli)
{
    return std::make_shared<Connection>(scheduler, sockfd, cli);
}

yrpc::coroutine::poller::Epoller* Connection::GetScheudler()
{
    return m_socket->scheduler;
}


}



