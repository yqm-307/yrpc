#include "RpcClientSession.h"

using namespace yrpc::rpc::detail;

bool RpcClientSession::RpcAsyncCall(std::shared_ptr<google::protobuf::Message> proto, RpcCallback func)
{
    if (m_regiseredmap.size() >= m_max_task) // 做一个限制，类似tcp窗口这种
        return false;

    // 临时的，内存自动释放，可以考虑改成可复用的，毕竟小内存频繁使用，可能导致内碎片增多
    auto req = yrpc::detail::protocol::RpcRequest::Create(yrpc::detail::protocol::type_C2S_RPC_CALL_REQ, proto.get()); 
    ProtoID id = req->GetProtoID();
    if (id == 0)
        return false;
    Future *obj = new Future(proto, func);      // 对于callback调用来说，handler中需要手动析构

    std::string bytearray;
    req->ToByteArray(bytearray);
    this->PendingToSend(bytearray.c_str(), bytearray.size());   // thread safe

    {
        yrpc::util::lock::lock_guard<yrpc::util::lock::Mutex> lock(m_regiseer_lock);
        m_regiseredmap.insert(Entry(id, obj));
    }
    if (OutLength() >= 4096)    // 4kb
        m_handler_cond.Notify();
    // m_timequeue.AddTask(yrpc::util::clock::nowAfter<yrpc::util::clock::ms>(yrpc::util::clock::ms(2000)), id); // 2s后超时    // 超时队列
    return true;
}

bool RpcClientSession::RpcSyncCall(Future &future)
{
    if (m_now_tasknum >= m_max_task)
        return false;

    auto req = yrpc::detail::protocol::RpcRequest::Create(yrpc::detail::protocol::type_C2S_RPC_CALL_REQ, future.m_send.get());
    ProtoID id = req->GetProtoID();
    if (id == 0)
        return false;
    // future.m_session = shared_from_this();
    std::string bytearray;
    req->ToByteArray(bytearray);
    this->PendingToSend(bytearray.c_str(), bytearray.size());

    {
        yrpc::util::lock::lock_guard<yrpc::util::lock::Mutex> lock(m_regiseer_lock);
        m_regiseredmap.insert(Entry(id, &future));
    }

    if (OutLength() >= 4096)
        m_handler_cond.Notify();
    m_timequeue.AddTask(yrpc::util::clock::nowAfter<yrpc::util::clock::ms>(yrpc::util::clock::ms(2000)), id); // 2s后超时

    m_now_tasknum++;
    return true;
}

void RpcClientSession::output_routinue()
{
    while (true)
    {
        if (!IsConnected()) // 断连补救措施
        {
            /* todo */
            ERROR("RpcClientSession::Handler error , info: disconnected!");
            break;
        }

        while (OutLength() == 0) // 注册但是未发送
            m_handler_cond.Wait();
        /// send
        this->SendAllByteArray();
    }
}

void RpcClientSession::input_routinue(const char *buffer, size_t len)
{
    this->AppendInputBuffer(buffer, len);
}

void RpcClientSession::Handler()
{
    while (true)
    {
        if (InLength() == 0)
        {
            yrpc::socket::YRSleep(m_io_context, 1); // 可以改成 cond_t
        }
        else
        {
            if (HasAProtocol())
            {
                std::string protobyte{};
                while (GetAProtocol(protobyte))
                {
                    yrpc::detail::protocol::YRPC_PROTOCOL type;
                    type = (yrpc::detail::protocol::YRPC_PROTOCOL)yrpc::util::protoutil::BytesToType<uint16_t>(protobyte.c_str());
                    yrpc::detail::protocol::RpcResponse rsp(type, protobyte);
                    Dispatch(rsp);
                }
            }
            else
                yrpc::socket::YRSleep(m_io_context, 1);
        }
    }
}


void RpcClientSession::Dispatch(const yrpc::detail::protocol::RpcResponse &rsp)
{
#define doif(ProtoType/*YRPC_PROTOCOL*/) case type_##ProtoType: {ProtoType##_Handler ( rsp );} break 
    using namespace yrpc::detail::protocol;
    auto type = rsp.GetProtoType();

    switch (type)
    {
    doif( S2C_HEARTBEAT_RSP );
    doif( S2C_RPC_CALL_RSP );
    default:
        FATAL("RpcClientSession::Dispatch() fatal , info: YRPC_PROTOCOL bad type[%d]",(uint16_t)type);
        break;
    }
#undef doif
}


/* todo : 完善 */
void RpcClientSession::S2C_RPC_CALL_RSP_Handler(const yrpc::detail::protocol::RpcResponse&rsp)
{
    ProtoID id = rsp.GetProtoID();

    Future *future_ptr = nullptr;
    {
        yrpc::util::lock::lock_guard<yrpc::util::lock::Mutex> lock(m_regiseer_lock);
        auto it = m_regiseredmap.find(id);
        future_ptr = it->second;
        if (it == m_regiseredmap.end())
        {
            ERROR("RpcClientSession::Handler fatal , info: proto id[%d] cannot find", id);
            return;
        }
        m_regiseredmap.erase(it);

    }

    auto msg = ProtocolFactroy::GetInstance()->Create(id);
    if (!rsp.ToProtoMsg(msg))
    {
        ERROR("RpcClientSession::S2C_RPC_CALL_RSP_Handler() error , info: Rpcresponse parse fatal!");
    }

    future_ptr->SetResult(msg); // 回调
    m_regiseredmap.erase(id);   // 删除
}

void RpcClientSession::S2C_HEARTBEAT_RSP_Handler(const yrpc::detail::protocol::RpcResponse&rsp)
{
    // DEBUG("todo :%s ",__FUNCTION__);
    ProtoID id = rsp.GetProtoID();
    auto it = m_regiseredmap.find(id); 
    INFO("heartbeat! ");
}