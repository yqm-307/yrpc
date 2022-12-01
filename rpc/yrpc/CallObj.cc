#include "CallObj.h"

using namespace yrpc::rpc::detail;

CallObj::CallObj(MessagePtr ptr, int id,uint32_t sid,YRPC_PROTOCOL type, CallResultFunc func)
    : m_req(ptr, sid,yrpc::detail::protocol::define::type_C2S_RPC_CALL_REQ),
      m_type_id(id),
      m_service_id(sid),
      m_call_type(type),
      m_callback(func)
{
    // 危险操作，临时使用原始指针
    // yrpc::detail::protocol::YProtocolGenerater generater(ptr,m_service_id, m_call_type);
    // generater.ToByteArray(m_req_bytearray);
}

void CallObj::SetResult(const std::string_view &view)
{
    m_rsp = std::move(Resolver(view));
    SetResult(m_rsp);
}

void CallObj::SetResult(const Resolver &res)
{

    yrpc::util::lock::lock_guard<Mutex> lock(m_lock);
    if (m_callback == nullptr)
    { // 同步唤醒
        m_rsp = res;
        m_cond_t.notify_all();
    }
    else
    { // 异步回调
        auto rsp = CreateARsp();
        res.ToProtoMsg(rsp);
        m_callback(rsp);
    }
}

CallObj::TYPE CallObj::GetResult(MessagePtr ret)
{

    if (m_callback != nullptr)
        return TYPE::RPC_CALL_IS_SYNC; // 不是异步调用
    if (m_rsp.IsEmpty())               // 尚未返回
        m_cond_t.wait();
    ret = ProtocolFactroy::GetInstance()->Create(m_type_id);
    return m_status;
}

uint32_t CallObj::GetID()
{
    return m_req.GetProtoID();
}

CallObj::MessagePtr CallObj::CreateAReq()
{
    return ProtocolFactroy::GetInstance()->Create(m_type_id);
}

CallObj::MessagePtr CallObj::CreateARsp()
{
    return ProtocolFactroy::GetInstance()->Create(m_type_id + 1);
}

const CallObj::Generater &CallObj::GetRequest()
{
    return m_req;
}
const CallObj::Resolver &CallObj::GetResponse()
{
    return m_rsp;
}