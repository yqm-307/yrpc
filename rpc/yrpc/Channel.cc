#include "Channel.h"


using namespace yrpc::rpc::detail;


void Channel::CloseInitFunc(const errorcode& e,const ConnPtr m_conn)
{
    INFO("Channel::CloseInitFunc(), info[errcode : %d]: peer:{ip:port} = {%s}\t",e.err(),m_conn->StrIPPort());
}

void Channel::ErrorInitFunc(const errorcode& e,const ConnPtr m_conn)
{
    INFO("Channel::ErrorCallback(), info[errocde : %d]:: peer:{ip:port} = {%s}\t",e.err(),m_conn->StrIPPort());
}








Channel::Channel()
    :m_conn(nullptr)
{
    this->SetCloseCallback([this](const errorcode& e){Channel::CloseInitFunc(e,this->m_conn);});
    this->SetErrorCallback([this](const errorcode& e){Channel::ErrorInitFunc(e,this->m_conn);});
}

Channel::Channel(ConnPtr newconn)
    :m_conn(newconn)
{
    DEBUG("Channel::Channel(), info: construction channel peer:{ip:port} = {%s}",newconn->StrIPPort().c_str());
}

Channel::~Channel()
{
    errorcode e("channel is destory",yrpc::detail::shared::ERR_TYPE_OK,0);
    m_closecallback(e);
    DEBUG("Channel::~Channel(), info: destory channel peer:{ip:port} = {%s}",m_conn->StrIPPort().c_str());
}



void Channel::Close()
{
    errorcode e("call func : Channel::Close,info: disconnection",yrpc::detail::shared::ERR_TYPE_OK,0);
    m_closecallback(e);
}       

bool Channel::IsClosed()
{
    return m_conn->IsClosed();
}

bool Channel::IsAlive()
{
    
}


/* send data to peer */
size_t Channel::Send(const Buffer& data)
{
    return m_conn->send(data);
}



/* send len byte to peer */
size_t Channel::Send(const char* data,size_t len)
{
    return m_conn->send(data,len);
}