#include "RpcSession.h"

using namespace yrpc::rpc::detail;




RpcSession::RpcSession(ChannelPtr channel,Epoller* loop)
    :m_channel(channel),
    m_remain((char*)calloc(sizeof(char),ProtocolMaxSize)),
    m_can_used(true)
{
}

RpcSession::~RpcSession()
{
    if( !IsClosed() )
    {
        Close();
    }
}



void RpcSession::Output(const char* data,size_t len)
{
    if(len ==  0)
        return;
    m_channel->Send(data,len);
}


void RpcSession::Input(const char* data,size_t len)
{
    // lock_guard<Mutex> lock(m_input_mutex);
    m_input_buffer.Append(data,len);
#ifdef YRPC_DEBUG
    m_byterecord.Addrecv_bytes(len);
#endif
}



std::vector<RpcSession::Protocol> RpcSession::GetProtocolsFromInput()
{
    std::vector<Protocol> protocols;
    while(true)
    {
        if (m_input_buffer.Has_Pkg())
        {
            Protocol proto;
            proto.data = m_input_buffer.GetAPck();
            if (proto.data.DataSize() == 0)
            {
                DEBUG("[YRPC][RpcSession::GetProtocolsFromInput]GetAPck error!");
                continue;
            }
            yrpc::detail::protocol::YProtocolResolver resolver(proto.data);
            if ( resolver.GetProtoType() < type_YRPC_PROTOCOL_CS_LIMIT )
            {// c2s 请求
                proto.t = Protocol::type::req;
                protocols.emplace_back(proto);
            }
            else
            {// s2c 响应
                proto.t = Protocol::type::rsp;
                protocols.emplace_back(proto);
            }
        }
        else
        {
            DEBUG("[YRPC][RpcSession::GetProtocolsFromInput] buffer size: %d",m_input_buffer.Length());
            break;
        }
    }
    return protocols;
}




size_t RpcSession::Append(const std::string_view pck)
{
    if(pck.size() == 0)
        return 0;

    // lock_guard<Mutex> lock(m_push_mutex);
    m_channel->Send(pck.data(),pck.size()); // 可能多线程同时send,需要加锁
    return pck.size();
}

size_t RpcSession::Append(const Buffer& bytearray)
{
    return m_channel->Send(bytearray);
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

void RpcSession::UpdataAllCallbackAndRunInEvloop()
{
    InitFunc();
    m_channel->UpdateAllCallbackAndRunInEvloop();    
}


void RpcSession::RecvFunc(const errorcode& e,Buffer& buff)
{
    m_byterecord.Addrecv_bytes(buff.DataSize());
    // 将数据保存到Buffer里
    if(e.err() == yrpc::detail::shared::ERR_NETWORK_RECV_OK)    // 正常接收
    {
        DEBUG("[YRPC]RpcSession::RecvFunc()  , info: recv succ %ld bytes!",buff.DataSize());
        DEBUG("[YRPC]RpcSession::RecvFunc()  , total recv %ld bytes!",m_byterecord.Getrecv_bytes());
        std::vector<Protocol> protos;
        {
            // lock_guard<Mutex> lock(m_input_mutex);
            Input(buff.Peek(),buff.DataSize());
            protos = GetProtocolsFromInput();
        }
        // do handle    
        HandleProtocol(protos);
    }
    else
    {
        ERROR("recv error!");
        // todo 错误处理
    }
}

void RpcSession::HandleProtocol(const std::vector<Protocol>& protocols)
{
    // while(protocols.size() > 0)
    for (auto it = protocols.begin(); it != protocols.end(); ++it)
    {
        auto proto = *it;
        yrpc::detail::protocol::YProtocolResolver resolver(proto.data); // todo 复用机制
        if ( resolver.GetProtoType() < type_YRPC_PROTOCOL_CS_LIMIT )
        {// c2s 请求
            if(m_ctoserver != nullptr)
                m_ctoserver(std::move(proto.data),shared_from_this());
            else
                NoneServerHandler();    
        }
        else
        {// s2c 响应
            if (m_stoclient != nullptr)
                m_stoclient(std::move(proto.data),shared_from_this());
            else
                NoneClientHandler();
        }
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
        DEBUG("[YRPC][RpcSession::SendFunc] info: send succ %ld bytes!",len);
        INFO("[YRPC][RpcSession::SendFunc] total send %ld bytes!",m_byterecord.Getsend_bytes());
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
    INFO("[YRPC][RpcSession::CloseFunc] info: Session Stop  peer = {%s}",m_channel->GetConnInfo()->GetPeerAddress().GetIPPort().c_str());
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
    WARN("[YRPC][RpcSession::NoneServerHandler] server handler is not set!");
}   

void RpcSession::NoneClientHandler()
{
    WARN("[YRPC][RpcSession::NoneClientHandler] client handler is not set!");
}

const Channel::Address& RpcSession::GetPeerAddress()
{
    return m_channel->GetConnInfo()->GetPeerAddress();
}

void RpcSession::AddPacket(const Protocol& pck)
{
    lock_guard<Mutex> lock(m_mutex_pck);    // 这里会和GetAllPak、GetAPack冲突,需要加锁
    m_pck_queue.push(pck);
}

int RpcSession::CallObj_AddObj(detail::CallObj::Ptr obj)
{
    auto it = m_call_map.find(obj->GetID());
    if ( it == m_call_map.end() )
    {
        return -1;
    }
    else
    {
        m_call_map.insert(std::make_pair(obj->GetID(),obj));
        return 1;
    }
}

int RpcSession::CallObj_DelObj(detail::CallObj::Ptr obj)
{
    auto it = m_call_map.find(obj->GetID());
    if ( it == m_call_map.end() )
    {
        return -1;
    }
    else
    {
        auto earse_node = m_call_map.erase(it);
        return 1;
    }
}


int RpcSession::SendACallObj(detail::CallObj::Ptr obj)
{
    int ret = -1;
    do{
        if (CallObj_AddObj(obj) < 0)
        {
            ERROR("[YRPC][RpcSession::SendACallObj] call map , id is repeat!");
            ret = -2;
            break;   
        }
        if (Append(obj->m_req) <= 0)
        {
            ret = -3;
            break;
        }
        ret = 1;
    }while(0);
    
    return ret;
}