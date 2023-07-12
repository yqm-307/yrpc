#pragma once
#include <iostream>
#include <google/protobuf/message.h>
#include <condition_variable>
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
    friend class yrpc::rpc::detail::RpcSession;
    typedef google::protobuf::Message       Message;
    typedef std::shared_ptr<Message>        MessagePtr;
    typedef yrpc::util::lock::Sem_t         Sem_t;
    typedef yrpc::util::lock::Mutex         Mutex;
    typedef yrpc::detail::protocol::YProtocolGenerater  Generater;  // 存储 request 并提供序列化
    typedef yrpc::detail::protocol::YProtocolResolver   Resolver;   // 存储 response bytearray 提供反序列化
    typedef yrpc::detail::protocol::define::YRPC_PROTOCOL   YRPC_PROTOCOL;
    typedef yrpc::rpc::detail::RPC_CALL_TYPE    TYPE;
    typedef bbt::buffer::Buffer             Buffer;
public:
    typedef std::shared_ptr<CallObj>        Ptr;
    typedef std::function<void(MessagePtr)> CallResultFunc;         // 


    ~CallObj();


    //////////////
    // 对外接口
    //////////////
    TYPE GetResult(MessagePtr);


    
    static Ptr Create(
            int reqtypeid,        // rpc请求的协议id
            int rsptypeid,                  // rpc响应的协议id
            Buffer&&,                       // rpc请求包的比特流
            YRPC_PROTOCOL,                  // 
            CallResultFunc=nullptr);
    
    
    CallObj(int reqtypeid,int rsptypeid,Buffer&& ptr,YRPC_PROTOCOL type,CallResultFunc func=nullptr);
private:
    CallObj() = delete;

    void    SetResult(const Buffer&);
    void    SetResult(const Resolver&);
    void    SetResult(Buffer&&);
    /**
     * @brief 创建一个和此次调用类型相同的 req 的google::protobuf对象 
     *  
     * @return * MessagePtr 如果失败返回nullptr
     */ 
    MessagePtr  CreateAReq();
    /**
     * @brief 创建一个和此次调用类型相同的 rsp 的google::protobuf对象 
     *  
     * @return * MessagePtr 如果失败返回nullptr
     */
    MessagePtr  CreateARsp();
    




    /**
     * @brief 获取包id(非0，如果错误返回0)
     * 
     * @return uint32_t 
     */
    uint32_t GetID();

private:
    Buffer              m_req;
    Resolver            m_rsp;
    uint32_t            m_service_id;   // 服务名
    YRPC_PROTOCOL       m_call_type{YRPC_PROTOCOL::type_YRPC_PROTOCOL_Done};
    CallResultFunc      m_callback; // 异步调用
    Sem_t               m_cond_t; // 通知用户完成
    Mutex               m_lock;
    TYPE                m_status;
    const int           m_typeid_req;
    const int           m_typeid_rsp;
};











}

}