#include "Channel.h"


using namespace yrpc::rpc::detail;

// 是否正在写
#define IsWriting(status) (!(status&Writing == 0)) 
// 是否正在读
#define IsReading(status) (!(status&Reading == 0))

void Channel::CloseInitFunc(const errorcode& e,const ConnPtr m_conn)
{
    INFO("Channel::CloseInitFunc(), info[errcode : %d]: peer:{ip:port} = {%s}\t",e.err(),m_conn->StrIPPort());
}

void Channel::ErrorInitFunc(const errorcode& e,const ConnPtr m_conn)
{
    INFO("Channel::ErrorCallback(), info[errocde : %d]:: peer:{ip:port} = {%s}\t",e.err(),m_conn->StrIPPort());
}


void Channel::SendInitFunc(const errorcode&e,size_t len,const ConnPtr m_conn)
{
    INFO("Channel::SendInitFunc(), info[errcode : %d]: peer:{ip:port} = {%s}\t",e.err(),m_conn->StrIPPort());
}

// void Channel::RecvInitFunc(const errorcode& e,Buffer&,const ConnPtr m_conn);





Channel::Channel()
    :m_conn(nullptr)
{
    this->SetCloseCallback([this](const errorcode& e){Channel::CloseInitFunc(e,this->m_conn);});
    this->SetErrorCallback([this](const errorcode& e){Channel::ErrorInitFunc(e,this->m_conn);});
    this->SetRecvCallback([this](const errorcode& e,Buffer& buff){this->m_recvcallback(e,buff);});
    this->SetSendCallback([this](const errorcode& e,size_t len){Channel::SendInitFunc(e,len,this->m_conn);});
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
    Send(data.peek(),data.ReadableBytes());
    // int n = m_conn->send(data);
    // errorcode e;
    // if(n > 0)
    // {
    //     e.set(yrpc::util::logger::format("send %d bytes",n),
    //         yrpc::detail::shared::YRPC_ERR_TYPE::ERRTYPE_NETWORK,       // code 类型 :网络
    //         yrpc::detail::shared::ERR_NETWORK::ERR_NETWORK_SEND_OK      // code :发送成功
    //     );    
    // }
    // else
    //     e.set("send fail",
    //         yrpc::detail::shared::YRPC_ERR_TYPE::ERRTYPE_NETWORK,
    //         yrpc::detail::shared::ERR_NETWORK::ERR_NETWORK_SEND_FAIL
    //     );
    // m_sendcallback(e,n);
    // return n;
}



/* send len byte to peer */
size_t Channel::Send(const char* data,size_t len)
{
    /**
     * 这里针对双缓冲进行的特殊处理，其实很简单，就是第一个缓冲正在被内核读写而不能改动。那么就让用户去读写另一个
     * 缓冲区，如果此时socket空闲了，那么就主动去send数据，这样就不会存在冲突了。但是不确保高效
     * 
     * 本段代码含义很简单：先写入buffer，然后判断socket是否可以写，可写则发送数据。
     */
    m_buff.GetCurrentBuffer().WriteString(data,len);
    if ( !IsWriting(m_status) )
    {
        int n = m_conn->send(m_buff.GetCurrentBuffer().peek(),m_buff.GetCurrentBuffer().ReadableBytes());   // 涉及m_conn的IO操作，可能导致挂起
        
        // io完成，状态转移
        m_buff.ChangeCurrent();
        m_status == Writing;

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
        return n;
    }
    else    
        return len;

    
}



// 防止代码污染
#ifdef IsWriting
#undef IsWriting
#endif


#ifdef IsReading
#undef IsReading
#endif


