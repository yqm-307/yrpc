#include "RpcSession.h"


using namespace yrpc::rpc::detail;






void RpcSession::Output(const char* data,size_t len)
{
    if(len ==  0)
        return;
    lock_guard<Mutex> lock(m_output_mutex);
    m_output_buffer.WriteString(data, len);
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
