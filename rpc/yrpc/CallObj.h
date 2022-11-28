#include <iostream>
#include <google/protobuf/message.h>
#include <condition_variable>
#include "../Util/Buffers.h"
#include "../Util/Locker.h"
#include "../msg/all.h"
#include "../protocol/all.h"
#include "Define.h"


namespace yrpc::rpc
{

class CallObjFactory;
class RpcClient;
namespace detail
{



/**
 * @brief 封装调用对象,屏蔽同步/异步调用的差异,可以说专门提供给服务调用方的
 * 1、提供一个公共的回调接口(为了屏蔽同步异步调用差距)
 * 
 */
class CallObj
{
    friend class yrpc::rpc::CallObjFactory;
    friend class yrpc::rpc::RpcClient;
    typedef google::protobuf::Message       Message;
    typedef std::shared_ptr<Message>        MessagePtr;
    typedef yrpc::util::buffer::Buffer      ByteArray;
    typedef yrpc::util::lock::Sem_t         Sem_t;
    typedef yrpc::util::lock::Mutex         Mutex;
    typedef yrpc::detail::protocol::YProtocolGenerater  Generater;  // 存储 request 并提供序列化
    typedef yrpc::detail::protocol::YProtocolResolver   Resolver;   // 存储 response bytearray 提供反序列化
    typedef yrpc::detail::protocol::define::YRPC_PROTOCOL   YRPC_PROTOCOL;
    typedef yrpc::rpc::detail::RPC_CALL_TYPE    TYPE;
public:
    typedef std::shared_ptr<CallObj>        Ptr;
    typedef std::function<void(MessagePtr)> CallResultFunc;         // 


    ~CallObj(){}


    //////////////
    // 对外接口
    //////////////
    TYPE GetResult(MessagePtr);

    template<typename MsgType>
    static Ptr Create(std::shared_ptr<MsgType>,int,uint32_t,YRPC_PROTOCOL,CallResultFunc=nullptr);
    
    
    CallObj(MessagePtr ptr,int id,uint32_t sid,YRPC_PROTOCOL type,CallResultFunc func=nullptr);
private:
    CallObj() = delete;

    void    SetResult(const std::string_view&);
    void    SetResult(const Resolver&);
    MessagePtr  CreateAReq();
    MessagePtr  CreateARsp();
    
    const Generater&    GetRequest();
    const Resolver&     GetResponse();
    /**
     * @brief 获取包id
     * 
     * @return uint32_t 
     */
    uint32_t GetID();

private:
    // Message*        m_message;  // 数据部分   
    Generater       m_req;
    std::string     m_req_bytearray;
    Resolver        m_rsp;
    std::string     m_rsq_bytearray;
    int             m_type_id;  // 类型id
    uint32_t        m_service_id;   // 服务名
    YRPC_PROTOCOL m_call_type{YRPC_PROTOCOL::type_YRPC_PROTOCOL_Done};
    const CallResultFunc    m_callback; // 异步调用
    Sem_t           m_cond_t; // 通知用户完成
    Mutex           m_lock;
    TYPE            m_status;
};




template <typename MsgType>
CallObj::Ptr CallObj::Create(std::shared_ptr<MsgType> ptr,int id,uint32_t name,YRPC_PROTOCOL type,CallResultFunc func)
{
    return std::make_shared<CallObj>(std::static_pointer_cast<Message>(ptr),id,name,type,func);
}







}

}