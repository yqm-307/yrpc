#include "CallObj.h"

using namespace yrpc::rpc::detail;

CallObj::CallObj(int reqtypeid, int rsptypeid, Buffer&& bytes, YRPC_PROTOCOL type, CallResultFunc func)
    : m_req(std::move(bytes)),
      m_call_type(type),
      m_callback(func),
      m_typeid_req(reqtypeid),
      m_typeid_rsp(rsptypeid)
{
    // 危险操作，临时使用原始指针
    // yrpc::detail::protocol::YProtocolGenerater generater(ptr,m_service_id, m_call_type);
    // generater.ToByteArray(m_req_bytearray);
}

CallObj::~CallObj()
{
    // DEBUG("~Callobj: 0x%x",this);
}

void CallObj::SetResult(const Buffer &view)
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
        if(res.ToProtoMsg(rsp))
        {
            m_callback(rsp);
        }
        else
        {
            ERROR("[YRPC][CallObj::SetResult][%d] parse failed! protoid:%d", y_scheduler_id, m_typeid_rsp);
        }
    }
}

void CallObj::SetResult(Buffer&&res)
{
    m_rsp = std::move(Resolver(std::move(res)));
    SetResult(m_rsp);
}



CallObj::TYPE CallObj::GetResult(MessagePtr ret)
{

    if (m_callback != nullptr)
        return TYPE::RPC_CALL_IS_SYNC; // 不是异步调用
    if (m_rsp.IsEmpty())               // 尚未返回
        m_cond_t.wait();
    ret = ProtocolFactroy::GetInstance()->Create(m_typeid_rsp);
    return m_status;
}

uint32_t CallObj::GetID()
{
    if (m_req.DataSize() < ProtocolHeadSize)
    {
        return 0;
    }
    Protocol_PckIdType id;
    memcpy((char*)&id,m_req.Peek()+(ProtocolHeadSize-sizeof(Protocol_PckIdType)),sizeof(Protocol_PckIdType));

    return id;
}

CallObj::MessagePtr CallObj::CreateAReq()
{
    return ProtocolFactroy::GetInstance()->Create(m_typeid_req);
}

CallObj::MessagePtr CallObj::CreateARsp()
{
    return ProtocolFactroy::GetInstance()->Create(m_typeid_rsp);
}


CallObj::Ptr CallObj::Create(
            int reqtypeid,        // rpc请求的协议id
            int rsptypeid,                  // rpc响应的协议id
            Buffer&&buf,                       // rpc请求包的比特流
            YRPC_PROTOCOL type,                  // 
            CallResultFunc f)
{
    return std::make_shared<CallObj>(reqtypeid,rsptypeid,std::forward<Buffer>(buf),type,f);
}