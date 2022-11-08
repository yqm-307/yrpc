#include "RpcSession.h"

using namespace yrpc::rpc::detail;




RpcSession::RpcSession(ChannelPtr channel,Epoller* loop)
    :m_remain((char*)calloc(sizeof(char),ProtocolMaxSize)),
    m_can_used(true),
    m_channel(channel)
{
    InitFunc();
}




void RpcSession::Output(const char* data,size_t len)
{
    if(len ==  0)
        return;
    m_channel->Send(data,len);
}


void RpcSession::Input(char* data,size_t len)
{
    // lock_guard<Mutex> lock(m_input_mutex);
    m_input_buffer.Append(data,len);
#ifdef YRPC_DEBUG
    m_byterecord.Addrecv_bytes(len);
#endif
}



void RpcSession::ProtocolMultiplexing()
{
    /**
     * 这个函数很重要
     * 1、解包然后判断数据包类型进行分类
     * 2、触发ToClient、ToServer分发给不同的处理handler
     */
    lock_guard<Mutex> lock(m_mutex_pck);    // 这里会和GetAllPak、GetAPack冲突,需要加锁
    while(true)
    {
        if (m_input_buffer.Has_Pkg())
        {
            Protocol proto;
            proto.data = m_input_buffer.GetAPck();
            if (proto.data.size() == 0)
                DEBUG("RpcSession::ProtocolMultiplexing(), info: GetAPck error!");
            yrpc::detail::protocol::YProtocolResolver resolver(proto.data);
            if ( resolver.GetProtoType() < type_YRPC_PROTOCOL_CS_LIMIT )
            {// c2s 请求
                proto.t = Protocol::type::req;
                if(m_ctoserver != nullptr)
                    m_ctoserver(proto.data);
                else    // 没有服务提供者的处理函数，返回错误信息
                    NoneServerHandler();    

            }
            else
            {// s2c 响应
                proto.t = Protocol::type::rsp;
                if (m_stoclient != nullptr)
                    m_stoclient(proto.data);
                else    // 没有客户端处理函数？存在这种可能吗。但是加上以防万一
                    NoneClientHandler();
            }
            m_pck_queue.push(proto);
        }
        else
        {
            break;
        }
    }
}





size_t RpcSession::Append(const std::string_view pck)
{
    if(pck.size() == 0)
        return 0;

    lock_guard<Mutex> lock(m_push_mutex);
    m_channel->Send(pck.data(),pck.size()); // 可能多线程同时send,需要加锁
    return pck.size();
}

size_t RpcSession::Append(const Buffer& bytearray)
{
    return Append(bytearray.View());
}


void RpcSession::InitFunc()
{
    m_channel->SetRecvCallback([this](const errorcode& e,Buffer& buff,const ConnPtr){
        this->RecvFunc(e,buff);
    });

    m_channel->SetSendCallback([this](const errorcode& e,size_t len,const ConnPtr){
        this->SendFunc(e,len);
    });

    m_channel->SetCloseCallback([this](const errorcode& e,const ConnPtr){
        this->CloseFunc(e);
    });

    m_channel->SetTimeOutCallback([this](){
        this->m_timeoutcallback();
    });

}

void RpcSession::RecvFunc(const errorcode& e,Buffer& buff)
{
    // 将数据保存到Buffer里
    if(e.err() == yrpc::detail::shared::ERR_NETWORK_RECV_OK)    // 正常接收
    {
        lock_guard<Mutex> lock(m_input_mutex);
        Input(buff.peek(),buff.ReadableBytes());
        ProtocolMultiplexing();     // 进行一次协议解析        
    }
    else
    {
        // todo 错误处理
    }
}


bool RpcSession::IsClosed()
{
    return m_channel->IsClosed();
}

void RpcSession::Close()
{
    m_channel->Close();
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
    // 关闭Session对外提供的API
    m_can_used.store(false);

    // 通知SessionManager
    if(m_closecb != nullptr)
        m_closecb(e,m_channel->GetConnInfo()->GetPeerAddress());
    
    if(e.err())
    {
        /* todo 完善错误码 */
    }
    INFO("RpcSession::CloseFunc() , info: Session Stop");
}


RpcSession::Protocol RpcSession::GetAPacket()
{
    lock_guard<Mutex> lock(m_mutex_pck);   
    if ( m_pck_queue.size() > 0 )
    {
        auto tmp = m_pck_queue.front();
        m_pck_queue.pop();
        return tmp;
    }    
    return Protocol();
}

RpcSession::PckQueue RpcSession::GetAllPacket()
{
    lock_guard<Mutex> lock(m_mutex_pck);   
    if ( m_pck_queue.size() > 0 )
    {
        PckQueue tmp;
        tmp.swap(m_pck_queue);
        return tmp;
    }
    return PckQueue();
}


bool RpcSession::HasPacket()
{
    lock_guard<Mutex> lock(m_mutex_pck);   
    return !m_pck_queue.empty();
}




void RpcSession::NoneServerHandler()
{

}   

void RpcSession::NoneClientHandler()
{

}