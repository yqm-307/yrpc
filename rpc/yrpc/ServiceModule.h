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
 */

class Service_Base
{   
    typedef yrpc::detail::protocol::define::YRPC_ErrCode RPC_ERRCODE;




    typedef yrpc::detail::ServiceMap ServiceMap;
    typedef yrpc::rpc::detail::RpcSession   RpcSession;
    typedef std::shared_ptr<RpcSession>     RpcSessionPtr;
    typedef yrpc::detail::protocol::YProtocolGenerater  Generater;
    typedef yrpc::detail::protocol::YProtocolResolver   Resolver;
    typedef yrpc::util::threadpool::ThreadPool<std::function<void()>> ThreadPool;
public:
    Service_Base(bool,int);
    ~Service_Base();

    static Service_Base* GetInstance(bool=false,int=-1);

    /**
     * @brief 再ServiceMap中找到对应的处理程序
     * 如果有本次 call 有异常，则处理异常问题，并返回异常信息。
     * 否则返回正常结果
     */
    void Dispatch(const std::string& ,RpcSessionPtr);

    /**
     * @brief 初始化本地服务,如果不初始化本地服务。则此进程仅被视为
     * 服务调用方，不会被视为服务提供方
     */
    void InitLocalService();
private:

    /////////////////////////
    ////// 错误处理函数///////
    /////////////////////////
    std::string DoErrHandler(RPC_ERRCODE,const std::string& info = "");

private:
    // servicemap 单例
    std::unique_ptr<ThreadPool>   m_pool;
    const bool               m_canToService;
};

}
