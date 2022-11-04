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


/**
 * @brief 封装调用对象,屏蔽同步/异步调用的差异,可以说专门提供给服务调用方的
 * 1、提供一个公共的回调接口(为了屏蔽同步异步调用差距)
 * 
 */
class CallObj
{
    friend CallObjFactory;
    friend RpcClient;
    typedef google::protobuf::Message       Message;
    typedef std::shared_ptr<Message>        MessagePtr;
    typedef yrpc::util::buffer::Buffer      ByteArray;
    typedef yrpc::util::lock::Sem_t         Sem_t;
    typedef yrpc::util::lock::Mutex         Mutex;
    typedef std::function<void(MessagePtr)> CallResultFunc;
    typedef yrpc::detail::protocol::YProtocolGenerater  Generater;  // 存储 request 并提供序列化
    typedef yrpc::detail::protocol::YProtocolResolver   Resolver;   // 存储 response bytearray 提供反序列化
    typedef yrpc::rpc::detail::RPC_CALL_TYPE    TYPE;
public:
    typedef std::shared_ptr<CallObj>        Ptr;


    ~CallObj();


    //////////////
    // 对外接口
    //////////////
    TYPE GetResult(MessagePtr);

    template<typename MsgType>
    static Ptr Create(MsgType,int,CallResultFunc);
    
    
private:
    CallObj() = delete;
    CallObj(MessagePtr ptr,int id,CallResultFunc func);

    void SetResult(const std::string_view&);
    void SetResult(const Resolver&);
    MessagePtr CreateAReq();
    MessagePtr CreateARsp();
    
    const Generater& GetRequest();
    const Resolver& GetResponse();
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
    const CallResultFunc    m_callback; // 异步调用
    Sem_t           m_cond_t; // 通知用户完成
    Mutex              m_lock;
    TYPE            m_status;
};




template <typename MsgType>
CallObj::Ptr CallObj::Create(MsgType ptr,int id,CallResultFunc func)
{
    return std::make_shared<CallObj>(ptr,id,func);
}








}