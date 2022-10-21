#include "RpcSession.h"

using namespace yrpc::rpc::detail;





/// 单线程内执行，无需加锁，任务注册加锁即可
void RpcSession::Output(const char* data,size_t len)
{
    // m_current_loop->AddTask();
    if(len ==  0)
        return;
    // lock_guard<Mutex> lock(m_output_mutex);
    m_channel->Send(data,len);

}


void RpcSession::Input(char* data,size_t len)
{
    lock_guard<Mutex> lock(m_input_mutex);
    m_input_buffer.Append(data,len);
#ifdef YRPC_DEBUG
    m_byterecord.Addrecv_bytes(len);
#endif
}



void RpcSession::ProtocolMultiplexing(const Buffer& buff)
{
    /**
     * 虽然这个函数很重要，但是主要功能还是解包然后判断数据包类型进行分类
     */
    if ( m_input_buffer.Has_Pkg() )
    {
        m_input_buffer.GetAReq();
    }
    else
    {

    }
    

}





size_t RpcSession::Append(const std::string_view pck)
{
    if(pck.size() == 0)
        return 0;
    lock_guard<Mutex> locak(m_push_mutex);
    m_current_loop->AddTask([&](void*){
        Output(pck.data(),pck.size());
    },nullptr);

    return pck.size();
}

size_t RpcSession::Append(const Buffer& bytearray)
{
    return Append(bytearray.View());
}


void RpcSession::InitFunc()
{
    m_channel->SetRecvCallback([this](const errorcode& e,Buffer& buff){
        this->RecvFunc(e,buff);
    });

    m_channel->SetSendCallback([this](const errorcode& e,size_t len){
        this->SendFunc(e,len);
    });

    m_channel->SetCloseCallback([this](const errorcode& e){
        this->CloseFunc(e);
    });

}

void RpcSession::RecvFunc(const errorcode& e,Buffer& buff)
{
    // 将数据保存到Buffer里
    if(e.err() == yrpc::detail::shared::ERR_NETWORK_RECV_OK)    // 正常接收
    {
        Input(buff.peek(),buff.ReadableBytes());
    }
    else
    {
        // todo 错误处理
    }
}

void RpcSession::SendFunc(const errorcode& e,size_t len)
{
    if( e.err() == yrpc::detail::shared::ERR_NETWORK_SEND_OK )
    {        
#ifdef YRPC_DEBUG
        m_byterecord.Addsend_bytes(len);
#endif
    }
    else
    {
        // todo 错误处理
        return;
    }

}

void RpcSession::CloseFunc(const errorcode& e)
{
    // todo 错误处理
}