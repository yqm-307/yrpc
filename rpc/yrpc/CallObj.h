#include <iostream>
#include <google/protobuf/message.h>
#include "../Util/Buffers.h"
#include "../Util/Locker.h"
#include "../msg/all.h"
#include "../protocol/all.h"

namespace yrpc::rpc
{

class CallObjFactory;
/**
 * @brief 封装调用对象,屏蔽同步/异步调用的差异
 */
class CallObj
{
    friend CallObjFactory;
    typedef google::protobuf::Message       Message;
    typedef std::shared_ptr<Message>        MessagePtr;
    typedef yrpc::util::buffer::Buffer      ByteArray;
    typedef yrpc::util::lock::Sem_t         Sem_t;
    typedef std::function<void(MessagePtr)> CallResultFunc;
    typedef yrpc::detail::protocol::YProtocolGenerater  Generater;
    typedef yrpc::detail::protocol::YProtocolResolver   Resolver;

public:
    typedef std::shared_ptr<CallObj>        Ptr;


    ~CallObj();

    template<typename MsgType>
    static Ptr Create(MsgType,int,CallResultFunc);

    //////////////
    // 对外接口
    //////////////
    bool IsAsync()
    { return !(m_callback == nullptr); }

    void CallBack(MessagePtr ptr)
    { assert(m_callback); m_callback(ptr); }

    MessagePtr GetRusult()
    { 
        if (m_rsp == nullptr)
            m_posix_cond_t.wait();
        else
            return m_rsp;         
    }

    
    
private:
    CallObj() = delete;
    CallObj(MessagePtr ptr,int id,CallResultFunc func)
        :m_req(ptr),
        m_callback(func),
        m_type_id(id)
    {
        // 危险操作，临时使用原始指针
        yrpc::detail::protocol::YProtocolGenerater generater(ptr.get(),yrpc::detail::protocol::define::type_C2S_RPC_CALL_REQ);
        m_bys.clear();
        generater.ToByteArray(m_bys);
        m_pck_id = generater.GetProtoID();  // 包id 
    }

    void SetResult(MessagePtr ptr)
    { 
        m_rsp = ptr; 
        m_posix_cond_t.notify_all();
    }

    MessagePtr CreateAReq()
    { return ProtocolFactroy::GetInstance()->Create(m_type_id); }

    MessagePtr CreateARsp()
    { return ProtocolFactroy::GetInstance()->Create(m_type_id+1); }
    
    /**
     * @brief 获取包id
     * 
     * @return uint32_t 
     */
    uint32_t GetID()
    {
        return m_
    }

private:
    // Message*        m_message;  // 数据部分   
    MessagePtr      m_req;      // 对象 
    MessagePtr      m_rsp;
    std::string     m_bys;      // 字节流
    std::string     m_result;   // 字节流
    int             m_type_id;  // 类型id
    int             m_pck_id;   // 包id
    const CallResultFunc  m_callback; // 异步调用
    Sem_t           m_posix_cond_t; // 通知用户完成
};




template <typename MsgType>
CallObj::Ptr CallObj::Create(MsgType ptr,int id,CallResultFunc func)
{
    return std::make_shared<CallObj>(ptr,id,func);
}








}