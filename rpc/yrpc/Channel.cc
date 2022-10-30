#include "Channel.h"


using namespace yrpc::rpc::detail;

// 是否正在写
#define IsWriting(status) (!(status&Writing == 0)) 
// 是否正在读
#define IsReading(status) (!(status&Reading == 0))
#define SetNoWriting(status) (status^Writing)
#define SetNoReading(status) (status^Reading)

void Channel::CloseInitFunc(const errorcode& e,const ConnPtr m_conn)
{ 
    INFO("Channel::CloseInitFunc(), info[errcode : %d]: peer:{ip:port} = {%s}\t",
        e.err(),
        m_conn->StrIPPort()); 
}

void Channel::ErrorInitFunc(const errorcode& e,const ConnPtr m_conn)
{ 
    INFO("Channel::ErrorCallback(), info[errocde : %d]:: peer:{ip:port} = {%s}\t",
        e.err(),
        m_conn->StrIPPort()); 
}


void Channel::SendInitFunc(const errorcode&e,size_t len,const ConnPtr m_conn)
{ 
    INFO("Channel::SendInitFunc(), info[errcode : %d]: peer:{ip:port} = {%s}\t",
        e.err(),
        m_conn->StrIPPort()); 
}

// void Channel::RecvInitFunc(const errorcode& e,Buffer&,const ConnPtr m_conn);





Channel::Channel()
    :m_conn(nullptr)
{
    InitFunc();
}

Channel::Channel(ConnPtr newconn)
    :m_conn(newconn)
{
    InitFunc();
    DEBUG("Channel::Channel(), info: construction channel peer:{ip:port} = {%s}",
            newconn->StrIPPort().c_str());
}

Channel::~Channel()
{
    errorcode e("channel is destory",yrpc::detail::shared::ERR_TYPE_OK,0);
    m_closecallback(e);
    DEBUG("Channel::~Channel(), info: destory channel peer:{ip:port} = {%s}",
            m_conn->StrIPPort().c_str());
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


size_t Channel::Send(const Buffer& data,Epoller* ep)
{
    return Send(data.peek(),data.ReadableBytes(),ep);
}



size_t Channel::Send(const char* data,size_t len,Epoller* ep)
{
    lock_guard<Mutex> lock(m_buff.m_lock);
    m_buff.GetCurrentBuffer().WriteString(data,len);
    if ( !IsWriting(m_status) )
    {
        m_status = m_status | Writing;
        ep->AddTask([=](void*ep){
            EpollerSend(m_buff.GetCurrentBuffer().peek(),m_buff.GetCurrentBuffer().DataSize(),(Epoller*)ep);
        },ep);
        m_buff.ChangeCurrent();
    }
    else    
        return len;

    return len;
}


void Channel::InitFunc()
{
    this->SetCloseCallback([this](const errorcode& e){Channel::CloseInitFunc(e,this->m_conn);});
    this->SetErrorCallback([this](const errorcode& e){Channel::ErrorInitFunc(e,this->m_conn);});
    this->SetRecvCallback([this](const errorcode& e,Buffer& buff){this->m_recvcallback(e,buff);});
    this->SetSendCallback([this](const errorcode& e,size_t len){Channel::SendInitFunc(e,len,this->m_conn);});
}



Channel::ChannelPtr Channel::Create(ConnPtr conn)
{
    return std::make_shared<Channel>(conn);
}

size_t Channel::EpollerSend(const char *data, size_t len,Epoller* ep)
{

    int n = m_conn->send(data,len);
    SetNoWriting(m_status);
    
    {// 线程安全的切换缓冲区,避免出现多线同时访问两个buffer的情况
        lock_guard<Mutex> lock(m_buff.m_lock);
        m_buff.GetIOBuffer().InitAll();
        m_buff.ChangeCurrent();
        if( m_buff.GetCurrentBuffer().DataSize() > 0 )  // 切换完,检测是否有数据，有则再注册
        {
            ep->AddTask([this](void*ep){
                EpollerSend(m_buff.GetCurrentBuffer().peek(),m_buff.GetCurrentBuffer().DataSize(),(Epoller*)ep);
            },ep);
        }
    }
    // 错误分析
    errorcode e;

    if (n > 0)
    {
        e.set(yrpc::util::logger::format("send %d bytes", n),
              yrpc::detail::shared::YRPC_ERR_TYPE::ERRTYPE_NETWORK,  // code 类型 :网络
              yrpc::detail::shared::ERR_NETWORK::ERR_NETWORK_SEND_OK // code :发送成功
        );
    }
    else
        e.set("send fail",
              yrpc::detail::shared::YRPC_ERR_TYPE::ERRTYPE_NETWORK,
              yrpc::detail::shared::ERR_NETWORK::ERR_NETWORK_SEND_FAIL);
    // 回调通知
    m_sendcallback(e, n);
}

// 防止代码污染
#undef IsWriting
#undef IsReading


