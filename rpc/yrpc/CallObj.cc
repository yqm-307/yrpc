#include "CallObj.h"

using namespace yrpc::rpc;

bool CallObj::IsAsync()
{
    return !(m_callback == nullptr);
}

void CallObj::CallBack(MessagePtr ptr)
{
    assert(m_callback);
    m_callback(ptr);
}

CallObj::MessagePtr CallObj::GetRusult()
{
    if (m_callback != nullptr)
        return nullptr;

    if (m_rsp.IsEmpty())
        m_posix_cond_t.wait();
    MessagePtr ret = ProtocolFactroy::GetInstance()->Create(m_type_id);
    return ret;
}

CallObj::CallObj(MessagePtr ptr, int id, CallResultFunc func)
    : m_req(ptr,yrpc::detail::protocol::define::type_C2S_RPC_CALL_REQ),
      m_callback(func),
      m_type_id(id)
{
    // 危险操作，临时使用原始指针
    yrpc::detail::protocol::YProtocolGenerater generater(ptr, yrpc::detail::protocol::define::type_C2S_RPC_CALL_REQ);
    generater.ToByteArray(m_req_bytearray);
}

void CallObj::SetResult(const std::string_view& view)
{
    assert(view.size() > 0);
    m_rsp.SetByteArray(view);
    m_posix_cond_t.notify_all();
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
