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


void Input(char*,size_t);



void RpcSession::ProtocolMultiplexing(const errorcode& e,const Buffer& buff)
{
    if(e.err() != yrpc::detail::shared::ERR_NETWORK_RECV_OK)
    {
        
    }
    else
        ERROR(e.what().c_str());
}





size_t RpcSession::PushPacket(const std::string& pck)
{
    
}

size_t RpcSession::PushByteArray(const Buffer& bytearray)
{

}