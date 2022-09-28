#pragma once
#include "ServerSingle.h"

namespace yrpc::rpc
{


class RpcServer
{
typedef yrpc::detail::ServiceFunc ServiceFunc;
typedef yrpc::detail::CodecFunc CodecFunc;
template<class Req>
using ReqPtr = std::shared_ptr<Req>;
template<class Rsp>
using RspPtr = std::shared_ptr<Rsp>;

public:
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


    /**
     * @brief loop开始
     */
    void start();

protected:

    /*一个线程一个scheduler、一个Acceptor、每个线程都是平行的，listen套接字设置 reuseaddr，内核负责负载均衡到来的连接*/
    void CreateSubServerThread(int stack_size,int maxqueue);


private:
    int port_;
    int socket_timeout_ms_;
    int connect_timeout_ms_;
    const int thread_num_;
    
    std::vector<yrpc::coroutine::poller::Epoller*> sches_;
    yrpc::coroutine::poller::Epoller* scheduler_;
    detail::ServerSingle* MainServer_;
    std::vector<detail::ServerSingle*> SubServers_;
    std::vector<std::thread*> Threads_;
    
    //每个服务 3个用户定义函数：1、连接时回调（提供缺省的）2、service handle（必须由用户定义）3、编、解码 handle
};



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