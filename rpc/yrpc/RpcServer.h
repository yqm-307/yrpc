#pragma once
#include "../network/TcpServer.h"
#include "../msg/servicemap.h"
#include "./Service.h"
#include "../Util/ThreadPool.h"
#include "SessionManager.h"
namespace yrpc::rpc
{


/**
 * @brief RpcServer 是rpc服务端启动函数，主要负责注册服务和初始化服务器配置
 * 1、注册服务: register_service，有两个重载，一个是自动生成id，一个是自己配置服务id
 * 2、ThreadPool配置: 是否设置线程池异步执行任务，如果不设置，默认为io线程内执行。
 * 3、
 */
class RpcServer
{
    typedef yrpc::detail::ServiceFunc           ServiceFunc;
    typedef yrpc::detail::CodecFunc             CodecFunc;
    typedef std::function<void()>               WorkFunc;
    typedef yrpc::detail::net::YAddress         Address;

    template <class Req>
    using ReqPtr = std::shared_ptr<Req>;
    template <class Rsp>
    using RspPtr = std::shared_ptr<Rsp>;
public:
    typedef yrpc::util::threadpool::ThreadPool<WorkFunc> ThreadPool;


    RpcServer(int port,size_t threadnum,std::string logpath = "server.log",int socket_timeout_ms=5000,int connect_timeout_ms=3000,int stack_size=64*1024,int maxqueue=65535);
    RpcServer()=delete;
    ~RpcServer();

    /**
     * @brief 注册Service到ServiceManager
     * 
     * @param string name 服务名，即该服务的名字，不允许重名服务存在，服务名hash生成id
     * @param function<google::protobuf::Message*(const_std::any)> func 服务处理函数,当连接建立并接收到protomsg时,将解析服务名并调用该服务; google::protobuf::Message*(ArgsPtr&)为服务原型函数,该服务需要
     * 返回一个message的指针（要求用户new创建，会由rpc框架回收内存）。传入ArgsPtr原型是 std::vector<std::any> 的智能指针。数组元素为msg对象顺序结构。
     * 如果元素中包含数组,则any需要被解释为vector,否则为单一元素;比如 int 就 转化为 int，int[] 转化为 vector<int>;
     * @param codec 编码解码处理函数，函数原型为 void(bool,std::any&,std::any&) 其中根据bool值的情况函数分为两种解释，且行为不同。
     * 如果bool 为 true,则执行为Parse行为;如果bool 为false,则执行为Serilize行为。
     */
    template<class ParamPackType,class ReturnPackType>
    void register_service(std::string name,ServiceFunc func);

    /**
     * @brief 指定id，方便双端定义协议
     * 
     * @tparam ParamPackType 
     * @tparam ReturnPackType 
     * @param name 
     * @param id 
     * @param func 
     */
    template<class ParamPackType,class ReturnPackType>
    void register_service(std::string name,int id,ServiceFunc func);


    void SetThreadPool(ThreadPool* pool);


    void Start();

    void Stop()
    { m_stop.store(true); }
private:    
    /**
     * @brief 处理一个完整的package
     *  
     * @param bytearray 一个完整的package的bytearray
     */
    void Dispatch(std::string& bytearray,detail::RpcSession::SessionPtr sess);


private:
    Address                     m_serv_addr;    // 服务器监听地址
    ThreadPool*                 m_pool;         // 线程池
    std::atomic_bool            m_stop{false};  //
};

// auto RpcServer::func = nullptr;   






template<class ParamPackType,class ReturnPackType>
void RpcServer::register_service(std::string name,ServiceFunc func)
{
    uint32_t ret = yrpc::detail::ServiceMap::GetInstance()->insert(name,func,
    /*解析：字节流转化为message对象。 序列化：message对象转化为字节流*/
    [](bool is_parse,std::any& arg1,std::any& arg2){
        if(is_parse)
            arg2 = yrpc::detail::Codec::ParseToMessage<ParamPackType>(std::any_cast<std::string_view&>(arg1));
        else //序列化
        {
            yrpc::detail::Codec::Serialize<ReturnPackType>(std::any_cast<google::protobuf::Message*>(arg1),std::any_cast<std::string&>(arg2));
        }
    });
    assert(ret >= 0);   //服务注册失败，大概率注册时导致的服务名冲突
}

template<class ParamPackType,class ReturnPackType>
void RpcServer::register_service(std::string name,int id,ServiceFunc func)
{
    uint32_t ret = yrpc::detail::ServiceMap::GetInstance()->insert(name,id,func,
    /*解析：字节流转化为message对象。 序列化：message对象转化为字节流*/
    [](bool is_parse,std::any& arg1,std::any& arg2){
        if(is_parse)
            arg2 = yrpc::detail::Codec::ParseToMessage<ParamPackType>(std::any_cast<std::string_view&>(arg1));
        else //序列化
        {
            yrpc::detail::Codec::Serialize<ReturnPackType>(std::any_cast<google::protobuf::Message*>(arg1),std::any_cast<std::string&>(arg2));
        }
    });
    assert(ret >= 0);   //服务注册失败，大概率注册时导致的服务名冲突
}


}