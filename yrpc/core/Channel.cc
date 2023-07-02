#include "Channel.h"


using namespace yrpc::rpc::detail;

// 是否正在写
#define IsWriting(status) ((status&Writing)) 
// 是否正在读
#define IsReading(status) ((status&Reading))
// 设置为不在写
#define SetNoWriting(status) (status&=~Writing)
// 设置为不在读
#define SetNoReading(status) (status&=~Reading)
#define SetIsWriting(status) (status|=Writing)
#define SetIsReading(status) (status|=Reading)

void Channel::CloseInitFunc(const errorcode& e)
{ 
    INFO("[YRPC][Channel::CloseInitFunc][%d] info[errcode : %d]: peer:{ip:port} = {%s}\t",
        y_scheduler_id,
        e.err(),
        m_conn->StrIPPort().c_str()); 
}

void Channel::ErrorInitFunc(const errorcode& e)
{ 
    INFO("[YRPC][Channel::ErrorCallback][%d] info[errocde : %d]:: peer:{ip:port} = {%s}\t",
        y_scheduler_id,
        e.err(),
        m_conn->StrIPPort().c_str()); 
}


void Channel::SendInitFunc(const errorcode&e,size_t len)
{ 
    INFO("[YRPC][Channel::SendInitFunc][%d] info[errcode : %d]: peer:{ip:port} = {%s}\t",
        y_scheduler_id,
        e.err(),
        m_conn->StrIPPort().c_str()); 
}

// void Channel::RecvInitFunc(const errorcode& e,Buffer&,const ConnPtr m_conn);





Channel::Channel()
    :m_conn(nullptr)
{
}

Channel::Channel(yrpc::detail::net::Connection::SPtr new_conn)
    :m_conn(new_conn),
    m_eventloop(new_conn->GetScheudler())
{
}

Channel::~Channel()
{
    errorcode e("channel is destory",yrpc::detail::shared::ERRTYPE_NOTHING,0);
    if (!m_is_closed)
    {
        m_conn->Close();
        if (m_closecallback == nullptr)
            CloseInitFunc(e);
        m_closecallback(e, m_conn);
    }

    DEBUG("[YRPC][Channel::~Channel][%d] info: destory channel peer:{ip:port} = {%s}",
            y_scheduler_id,
            m_conn->StrIPPort().c_str());
}


void Channel::SetAcceptor(Acceptor::SPtr acceptor)
{
    m_acceptor = acceptor;
}

void Channel::SetConnector(Connector::SPtr connector)
{
    m_connector = connector;
}

void Channel::Init(Acceptor::SPtr acceptor, Connector::SPtr connector)
{
    m_acceptor = acceptor;
    m_connector = connector;
}


void Channel::Close()
{
    errorcode e("call func : Channel::Close,info: disconnection",yrpc::detail::shared::ERRTYPE_NOTHING,0);
    m_conn->Close();
    // m_closecallback(e,m_conn); // 重复调用了?
    m_is_closed = true;
}       

bool Channel::IsClosed()
{
    return m_conn->IsClosed();
}

bool Channel::IsAlive()
{
    return !m_conn->IsClosed();
}


size_t Channel::Send(const Buffer& data)
{
    return Send(data.Peek(),data.ReadableBytes());
}



size_t Channel::Send(const char* data,size_t len)
{
    lock_guard<Mutex> lock(m_mutex_buff);
    m_buffer.WriteString(data, len);
    if (!IsWriting(m_status)) // 没有正在发送数据
    {
        SetIsWriting(m_status);
        // m_status = m_status | Writing;
        Buffer IObuff;
        IObuff.Swap(m_buffer); //
        m_eventloop->AddTask([this, IObuff](void *)
        { 
            EpollerSend(IObuff.Peek(), IObuff.DataSize()); 
        });
    }
    return len;
}


void Channel::InitFunc()
{
    m_conn->setOnRecvCallback([this](const errorcode& e,Buffer& data){
        assert(this->m_recvcallback!=nullptr);
        this->m_recvcallback(e,data,this->m_conn);
    });

    m_conn->setOnTimeoutCallback([this](Socket* socket){
        this->m_timeoutcallback(socket);
    });

    m_conn->setOnCloseCallback([this](const errorcode& e,const ConnPtr p){
        this->m_closecallback(e, p);
    });

    m_conn->StartRecvFunc();
}



Channel::ChannelPtr Channel::Create(ConnPtr conn)
{
    return std::make_shared<Channel>(conn);
}


void Channel::UpdateAllCallbackAndRunInEvloop()
{
    InitFunc();
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
        //try to send
        if (m_buffer.DataSize() > 0)
        {
            // m_buffer.WriteString(m_buffer.Peek(), m_buffer.DataSize());
            SetIsWriting(m_status);
            Buffer IObuff;
            IObuff.Swap(m_buffer); //
            m_eventloop->AddTask([this, IObuff](void *)
            { 
                EpollerSend(IObuff.Peek(), IObuff.DataSize()); 
            });
        }
    }
    // 错误分析
    errorcode e;

    if (n > 0)
    {
        e.set(bbt::log::format("send %d bytes", n),
              yrpc::detail::shared::YRPC_ERR_TYPE::ERRTYPE_NETWORK,  // code 类型 :网络
              yrpc::detail::shared::ERR_NETWORK::ERR_NETWORK_SEND_OK // code :发送成功
        );
    }
    else
        e.set("send fail",
              yrpc::detail::shared::YRPC_ERR_TYPE::ERRTYPE_NETWORK,
              yrpc::detail::shared::ERR_NETWORK::ERR_NETWORK_SEND_FAIL);
    m_sendcallback(e, n, m_conn);
}

#undef IsWriting
#undef IsReading


