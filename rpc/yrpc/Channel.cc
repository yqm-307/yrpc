#include "Channel.h"


using namespace yrpc::rpc::detail;

// 是否正在写
#define IsWriting(status) (!(status&Writing == 0)) 
// 是否正在读
#define IsReading(status) (!(status&Reading == 0))
#define SetNoWriting(status) (status^Writing)
#define SetNoReading(status) (status^Reading)

void Channel::CloseInitFunc(const errorcode& e)
{ 
    INFO("Channel::CloseInitFunc(), info[errcode : %d]: peer:{ip:port} = {%s}\t",
        e.err(),
        m_conn->StrIPPort().c_str()); 
}

void Channel::ErrorInitFunc(const errorcode& e)
{ 
    INFO("Channel::ErrorCallback(), info[errocde : %d]:: peer:{ip:port} = {%s}\t",
        e.err(),
        m_conn->StrIPPort().c_str()); 
}


void Channel::SendInitFunc(const errorcode&e,size_t len)
{ 
    INFO("Channel::SendInitFunc(), info[errcode : %d]: peer:{ip:port} = {%s}\t",
        e.err(),
        m_conn->StrIPPort().c_str()); 
}

// void Channel::RecvInitFunc(const errorcode& e,Buffer&,const ConnPtr m_conn);





Channel::Channel()
    :m_conn(nullptr)
{
    InitFunc();
}

Channel::Channel(ConnPtr newconn,Epoller* ep)
    :m_conn(newconn),
    m_eventloop(ep)
{
    assert(m_eventloop != nullptr);
    InitFunc();
    DEBUG("Channel::Channel(), info: construction channel peer:{ip:port} = {%s}",
            newconn->StrIPPort().c_str());
}

Channel::~Channel()
{
    errorcode e("channel is destory",yrpc::detail::shared::ERR_TYPE_OK,0);
    if (!m_is_closed)
    {
        if (m_closecallback == nullptr)
            CloseInitFunc(e);
        m_closecallback(e, m_conn);
    }

    DEBUG("Channel::~Channel(), info: destory channel peer:{ip:port} = {%s}",
            m_conn->StrIPPort().c_str());
}



void Channel::Close()
{
    errorcode e("call func : Channel::Close,info: disconnection",yrpc::detail::shared::ERR_TYPE_OK,0);
    m_closecallback(e,m_conn);
    m_is_closed = true;
}       

bool Channel::IsClosed()
{
    return m_conn->IsClosed();
}

bool Channel::IsAlive()
{
    
}


size_t Channel::Send(const Buffer& data)
{
    return Send(data.peek(),data.ReadableBytes());
}



size_t Channel::Send(const char* data,size_t len)
{

    m_buffer.WriteString(data,len);
    
    m_mutex_buff.lock();
    if ( !IsWriting(m_status) ) // 没有正在发送数据
    {    

        m_status = m_status | Writing;
        std::shared_ptr<Buffer> IObuffptr = std::make_shared<Buffer>();
        IObuffptr->swap(m_buffer);  // 
        m_eventloop->AddTask([=](void*){
            EpollerSend(IObuffptr->peek(),IObuffptr->DataSize());
        });
    }

    m_mutex_buff.unlock();
    return len;
}


void Channel::InitFunc()
{
    m_conn->setOnRecvCallback([this](const errorcode& e,Buffer& data){
        assert(this->m_recvcallback!=nullptr);
        this->m_recvcallback(e,data,this->m_conn);
    });

    m_conn->setOnTimeoutCallback([this](){
        this->m_timeoutcallback();
    });
}



Channel::ChannelPtr Channel::Create(ConnPtr conn,Epoller* ep)
{
    return std::make_shared<Channel>(conn,ep);
}

void Channel::EpollerSend(const char *data, size_t len)
{

    int n = m_conn->send(data,len);
    {
        lock_guard<Mutex> lock(m_mutex_buff);
        SetNoWriting(m_status);
        /**
         * 因为发送事件的注册是工作线程调用send驱动的，如果buffer有数据，但是send不被调用。
         * 就会导致数据积压在buffer中。所以我添加一个主动检测机制，发送完毕后，检查缓冲区
         * 是否还有待发送数据，如果没有就可以退出。如果有，就直接注册一个发送事件
         * 
         * 思考另一个方法，就是保证注册发送事件的顺序。
         * 
         * 这样多线程send也不会出错
         */
        if ( m_buffer.DataSize() > 0 ) 
        {
            Send(m_buffer);
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
    m_sendcallback(e, n,m_conn);
}

// 防止代码污染
#undef IsWriting
#undef IsReading


