#include "RpcSession.h"
#include "ServiceModule.h"
// #include "yrpc/protocol/all.h"
using namespace yrpc::rpc::detail;




RpcSession::RpcSession(ChannelPtr channel,Epoller* loop)
    :m_channel(channel),
    m_remain((char*)calloc(sizeof(char),ProtocolMaxSize)),
    m_can_used(true),
    m_last_active_time(yrpc::util::clock::now<yrpc::util::clock::ms>()),
    m_handshake_time_isstop(true),
    m_current_loop(loop)
{
}

RpcSession::~RpcSession()
{
    if( !IsClosed() )
    {
        Close();
        DEBUG("[YRPC][RpcSession::~RpcSession][%d] RpcSession is destory!", y_scheduler_id);
    }
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
                DEBUG("[YRPC][RpcSession::GetProtocolsFromInput][%d] GetAPck error!", y_scheduler_id);
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
            // DEBUG("[YRPC][RpcSession::GetProtocolsFromInput] buffer size: %d", m_input_buffer.Length());
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

    m_channel->SetTimeOutCallback([this](Socket* socket){
        this->TimeOut(socket);
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
    if(e.err() == yrpc::detail::shared::ERR_NETWORK_RECV_OK)    // 正常接收
    {
        UpdateTimeout();    // 刷新超时时间
        std::vector<Protocol> protos;
        {
            m_input_buffer.Append(buff.Peek(), buff.DataSize());
#ifdef YRPC_DEBUG
            m_byterecord.Addrecv_bytes(buff.DataSize());
#endif
            protos = GetProtocolsFromInput();
        }
        // DEBUG("[YRPC][RpcSession::RecvFunc] recv %ld byte. total recv %ld bytes!", buff.DataSize(), m_byterecord.Getrecv_bytes());
        HandleProtocol(protos);
    }
    else
    {
        ERROR("[YRPC][RpcSession::RecvFunc][%d] recv error!", y_scheduler_id);
        // todo 错误处理
    }
}

void RpcSession::HandleProtocol(const std::vector<Protocol>& protocols)
{
    for (auto it = protocols.begin(); it != protocols.end(); ++it)
    {
        auto proto = *it;
        yrpc::detail::protocol::YProtocolResolver resolver(proto.data); // todo 复用机制
        if ( resolver.GetProtoType() < type_YRPC_PROTOCOL_CS_LIMIT )
        {// c2s 请求
            Dispatch(std::move(proto.data), shared_from_this());
        }
        else
        {// s2c 响应
            CallObj_CallResult(std::move(proto.data));
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
        UpdateTimeout();
#ifdef YRPC_DEBUG
        m_byterecord.Addsend_bytes(len);
        DEBUG("[YRPC][RpcSession::SendFunc][%d] send %ld bytes. total send %ld bytes!", y_scheduler_id, len, m_byterecord.Getsend_bytes());
#endif
    }
    else
    {
        // todo 错误处理
        ERROR("[YRPC][RpcSession::SendFunc][%d] %s", y_scheduler_id, e.what().c_str());
        return;
    }

}

void RpcSession::CloseFunc(const errorcode& e)
{
    // 关闭Session对外提供的API
    m_can_used.store(false);

    // 通知SessionManager
    if(m_closecb != nullptr)
        m_closecb(e, shared_from_this());
    
    if(e.err())
    {
        /* todo 完善错误码 */
    }
    INFO("[YRPC][RpcSession::CloseFunc][%d] info: Session Stop  peer = {%s}", y_scheduler_id, m_channel->GetConnInfo()->GetPeerAddress().GetIPPort().c_str());
}

void RpcSession::UpdateTimeout()
{
    m_last_active_time = yrpc::util::clock::now<yrpc::util::clock::ms>();
}

void RpcSession::TimeOut(Socket* socket)
{
    auto timenow_ms = yrpc::util::clock::now<yrpc::util::clock::ms>();
    auto alrealy_timeout_ms = (timenow_ms - m_last_active_time).count();
    if (alrealy_timeout_ms >= YRPC_SESSION_TIMEOUT)
    {
        yrpc::detail::shared::errorcode err("", yrpc::detail::shared::ERRTYPE_YCO, yrpc::detail::shared::ERR_YCO::ERR_YCO_TIMEOUT);
        m_timeoutcallback(err, shared_from_this());
    }
    else
    {
        assert(socket->scheduler != nullptr);
        socket->scheduler->AddSocketTimer(socket);
    }
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

const Channel::Address& RpcSession::GetPeerAddress()
{
    return m_channel->GetConnInfo()->GetPeerAddress();
}

void RpcSession::AddPacket(const Protocol& pck)
{
    lock_guard<Mutex> lock(m_mutex_pck);    // 这里会和GetAllPak、GetAPack冲突,需要加锁
    m_pck_queue.push(pck);
}

int RpcSession::CallObj_AddObj(detail::CallObj::Ptr obj) /* 发起一次调用时做一次Add*/
{

    lock_guard<Mutex> lock(m_mutex_call_map);
    auto it = m_call_map.find(obj->GetID());
    if ( it == m_call_map.end() )
    {
        m_call_map.insert(std::make_pair(obj->GetID(),obj));
        return 1;
    }
    else
    {
        return -1;
    }
}


int RpcSession::CallObj_DelObj(Protocol_PckIdType id) /* 处理一次调用后删除掉 */
{
    lock_guard<Mutex> lock(m_mutex_call_map);
    auto it = m_call_map.find(id);
    if ( it == m_call_map.end() )
    {
        return -1;
    }
    else
    {
        m_call_map.erase(it);
        return 1;
    }
}


int RpcSession::SendACallObj(detail::CallObj::Ptr obj)
{
    int ret = -1;
    do{
        if (CallObj_AddObj(obj) < 0)
        {
            ERROR("[YRPC][RpcSession::SendACallObj][%d] call map , id is repeat!", y_scheduler_id);
            ret = -2;
            break;   
        }
        if (Append(obj->m_req) <= 0)
        {
            ret = -3;
            CallObj_DelObj(obj->GetID());
            break;
        }
        ret = 1;
    }while(0);
    
    return ret;
}

void RpcSession::Dispatch(Buffer&& buf, SessionPtr sess)
{
    yrpc::rpc::detail::Service_Base::GetInstance()->Dispatch(std::forward<Buffer>(buf), sess);
}

int RpcSession::CallObj_CallResult(Buffer&& buf)
{
    yrpc::detail::protocol::YProtocolResolver rsl(buf);
    auto protoid = rsl.GetProtoID();
    // 获取包id,很重要，to select callobjmap
    {
        yrpc::util::lock::lock_guard<Mutex> lock(m_mutex_call_map);
        auto it = m_call_map.find(protoid);
        if (it == m_call_map.end())
        {
            ERROR("[YRPC][RpcSession::CallObj_CallResult][%d] info: cann`t find package id", y_scheduler_id);
        }
        else
        {
            it->second->SetResult(rsl);    //设置结果
        }
    }
    CallObj_DelObj(protoid);
}

void RpcSession::StartHandShakeTimer(const SessionHandShakeTimeOutCallback& handshake_timeout_func, int timeout_ms)
{
    using namespace yrpc::detail::shared;
    if( !m_handshake_time_isstop )
    {
        WARN("[YRPC][RpcSession::StartHandShakeTimer][%d] repeat start shake timer!", y_scheduler_id);
        return;
    }
    m_handshake_time_isstop.exchange(false);
    m_handshake_time_isstop = m_current_loop->AddTimer([this,handshake_timeout_func](){
        yrpc::detail::shared::errorcode e(
            "session handshake timeout!",
            ERRTYPE_HANDSHAKE,
            ERR_HANDSHAKE::ERR_HANDSHAKE_TIMEOUT);
        if( !m_handshake_time_isstop )
        {
            handshake_timeout_func(e, shared_from_this());
        }
    },timeout_ms);
}

void RpcSession::StopHandShakeTimer()
{
    m_handshake_time_isstop.exchange(true);
}