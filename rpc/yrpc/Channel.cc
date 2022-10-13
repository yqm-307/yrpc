#include "Channel.h"


using namespace yrpc::rpc::detail;


Channel::Channel()
    :m_conn(nullptr)
{}

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