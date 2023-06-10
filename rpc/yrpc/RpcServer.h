#pragma once
#include "../network/TcpServer.h"
#include "../msg/servicemap.h"
#include "../Util/ThreadPool.h"
#include "SessionManager.h"
#include "../msg/ProtocolFactory.h"
namespace yrpc::rpc
{


/**
 * @brief RpcServer 是rpc服务端启动函数，主要负责注册服务和初始化服务器配置
 * 1、注册服务: register_service，有两个重载，一个是自动生成id，一个是自己配置服务id
 * 2、ThreadPool配置: 是否设置线程池异步执行任务，如果不设置，默认为io线程内执行。
 * 3、
 */
class RpcServer : bbt::noncopyable
{
    typedef yrpc::detail::ServiceFunc           ServiceFunc;
    typedef yrpc::detail::CodecFunc             CodecFunc;
    typedef std::function<void()>               WorkFunc;
    typedef yrpc::detail::net::YAddress         Address;
    typedef google::protobuf::Message           Message;
    typedef std::shared_ptr<Message>            MessagePtr;

    template <class Req>
    using ReqPtr = std::shared_ptr<Req>;
    template <class Rsp>
    using RspPtr = std::shared_ptr<Rsp>;
public:
    typedef yrpc::util::threadpool::ThreadPool<WorkFunc> ThreadPool;


    RpcServer();
    ~RpcServer();

    /**
     * @brief 设置服务器地址，一定要在启动前设置(移动语义)
     */
    void SetAddress(Address&&);
    /**
     * @brief 设置服务器地址，一定要在启动前设置
     */
    void SetAddress(const Address&);



    static RpcServer* GetInstance()
    {
        static RpcServer* _istn;
        if (_istn == nullptr)
            _istn = new RpcServer();
        return _istn; 
    }




    
    /* 启动服务，立即返回 */
    void Start();
    void Stop()
    { m_stop.store(true); }
private:
    Address                     m_serv_addr;    // 服务器监听地址
    std::atomic_bool            m_stop{false};  
};
}