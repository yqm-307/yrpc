#include "./connection.h"

namespace yrpc::detail::ynet
{

Connection::Connection(yrpc::coroutine::poller::Epoller* scheduler,RoutineSocket* sockfd,const YAddress& cli)
    :conn_status_(connecting),
    schedule_(scheduler),
    cliaddr_(cli),
    socket_(sockfd)
{
    conn_status_ = connected;
    schedule_->AddTask([this](void*ptr){recvhandler();},nullptr);   //注册recv handler
    INFO("Connection::Connection() , info: connect success ! peer addr : %s",cli.GetIPPort().c_str());
}
Connection::~Connection()
{

}


size_t Connection::send(const char* data,size_t len)
{
    int n=0;
    if(conn_status_ == connected)
    {

        n = yrpc::socket::YRSend(*socket_,data,len,0);    
        if(n<0)
        {
            int errorp = yrpc::util::tcp::GetSockErrCode(socket_->sockfd_);
            if(errorp == EPIPE) //对端断开
                Close();
            ERROR("Connection::send() error , socket error %d",errorp);
        }
    }
    else if(conn_status_ == disconnect)
    {
        ERROR("Connection::send() error , conn_status is disconnect!");
        return -1;
    }
    return n;
}

size_t Connection::recv(char* buffer,size_t buflen)
{
    int n = 0;
    if (conn_status_ == connected)
    {
        while (1)
        {
            n = yrpc::socket::YRRecv(*socket_, buffer, buflen, 0);
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
    else if (conn_status_ == disconnect)
    {
        ERROR("Connection::recv() error , conn_status is disconnect!");
        return -1;
    }
    return n;
}

void Connection::Close()
{
    conn_status_ = disconnect;
    INFO("Connection::Close() : disconnect");
    if(closecb_ != nullptr)
        closecb_(shared_from_this());
}

void runhandle()
{
    
}


void Connection::recvhandler()
{

    // yrpc::util::buffer::Buffer buffer;
    char* buffer = new char[4096];
    while(conn_status_)
    {
        memset(buffer,'\0',sizeof(buffer));
        int n = recv(buffer,sizeof(buffer));

        if(n > 0) 
        {
            DEBUG("Connection::recvhandler() debug , info: from internet recv %d bytes",n);
            if(onrecv_)
                onrecv_(buffer,n);
            else
                ERROR("Connection::recvhandler() error , info: recv handler is illegal!");
        }
    }

}

}



