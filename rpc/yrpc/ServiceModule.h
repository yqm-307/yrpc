#pragma once
#include "../msg/servicemap.h"
#include "RpcSession.h"
#include "../protocol/YProtocolResolver.h"
#include "../protocol/YProtocolGenerater.h"

namespace yrpc::rpc::detail
{




/**
 * @brief yrpc 协议处理基础机制。
 *  1、服务不存在
 *  2、熔断
 *  3、拒绝
 *  4、服务处理错误
 * 
 *  Service_Base 是全局单例，是 yrpc 最基础的协议处理机制（兜底的）
 *  除了系统定义的，
 * 
 *  Service_Base 中的 service handle 不会注册在 ServiceMap中。可以理解为这里的Dispatch相当于
 *  过滤器一般，将正常被用户定义的协议传递给用户，用户没有自定义处理程序的，走yrpc的处理流程
 */

class Service_Base
{   
    typedef yrpc::detail::protocol::define::YRPC_ErrCode RPC_ERRCODE;
    typedef yrpc::detail::protocol::define::YRPC_PROTOCOL   YRPC_PROTOCOL;

    typedef yrpc::util::buffer::Buffer                      Buffer;
    typedef std::shared_ptr<RpcSession>                     RpcSessionPtr;
    typedef std::shared_ptr<google::protobuf::Message>      MessagePtr;
    typedef yrpc::detail::ServiceMap                        ServiceMap;
    typedef yrpc::detail::protocol::YProtocolGenerater      Generater;
    typedef yrpc::detail::protocol::YProtocolResolver       Resolver;
    typedef yrpc::util::threadpool::ThreadPool<std::function<void()>> ThreadPool;
    typedef yrpc::detail::protocol::ProtocolHead            ProtocolHead;
public:
    Service_Base(bool,int);
    ~Service_Base();

    static Service_Base* GetInstance(bool=false,int=-1);

    /**
     * @brief 再ServiceMap中找到对应的处理程序
     * 如果有本次 call 有异常，则处理异常问题，并返回异常信息。
     * 否则返回正常结果
     */
    void Dispatch(Buffer&& ,RpcSessionPtr);

    void SendPacket(RpcSessionPtr,MessagePtr,const ProtocolHead&,YRPC_PROTOCOL);

    /**
     * @brief 初始化本地服务,如果不初始化本地服务。则此进程仅被视为
     * 服务调用方，不会被视为服务提供方
     */
    // void InitLocalService();
private:

    /////////////////////////
    ////// 错误处理函数///////
    /////////////////////////
    MessagePtr DoErrHandler(RPC_ERRCODE,const std::string& info = "");

private:
    // servicemap 单例
    std::unique_ptr<ThreadPool>   m_pool;
    const bool               m_canToService;
};

}
