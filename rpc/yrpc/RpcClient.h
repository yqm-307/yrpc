/**
 * @file RpcClient.h
 * @author yqm-307 (979336542@qq.com)
 * @brief RpcClient 是YRPC 中最常用的类之一，通过该类才可以发起调用
 * @version 0.1
 * @date 2022-06-20
 * 
 * @copyright Copyright (c) 2022
 */


#pragma once
#include "../msg/servicemap.h"
#include "./Service.h"
#include "./RpcSession.h"
#include "./CallResult.h"
#include "./RpcClientSession.h"
#include "./SessionManager.h"

namespace yrpc::rpc
{

/**
 * @brief RpcClient 是调用方最重要的类，通过该类才可以发起调用
 */
class RpcClient
{
    typedef detail::SessionManager                  SessionManager;
    typedef detail::SessionManager::SessionID       SessionID;
    // using net = yrpc::detail::net;
    typedef detail::RpcSession                      RpcSession;
    typedef yrpc::detail::shared::errorcode         errcode;
public:

    /* Result 的 指针类型 */
    // template<class S,class R> using ResultPtr = std::shared_ptr<Result<S,R>>;
    
    /* Rsp(响应报文) 的 protomsg指针类型 */
    template<class Response>   using RspPtr = std::shared_ptr<Response>;
    
    /* Req(请求报文) 的 protomsg指针类型 */
    template<class SType>   using ReqPtr = std::shared_ptr<SType>;


public:

    /**
     * @brief 构造一个RpcClient
     * 
     * @param ip    服务提供方IP地址
     * @param port  服务提供方端口
     * @param logpath       日志名
     * @param stack_size    协程栈大小，默认128kb
     * @param maxqueue      协程数上限
     */
    RpcClient(std::string ip,int port,std::string logpath=InitLogName,int stack_size=128*1024,int maxqueue=65535);
    RpcClient(yrpc::detail::net::YAddress servaddr_,std::string logpath=InitLogName,int stack_size=128*1024,int maxqueue=65535);
    ~RpcClient();   //todo,退出不安全，在Client关闭时，应该先让所有请求都返回或者直接失败
    
    void close();
    bool isclose();

    
    // template<class SendType,class RecvType>
    // bool async_call_long(std::string name,ReqPtr<SendType> send,std::function<void(RecvType&)> f);

    // template<class SendType,class RecvType>
    // ResultPtr<SendType,RecvType> result_long(std::string name,ReqPtr<SendType> send);


    bool async_call(std::string name,std::shared_ptr<google::protobuf::Message> send,yrpc::rpc::detail::RpcCallback f);

    // 提交调用
    // bool Updata();
private:
    static void run(void*);
    void NewConnection(yrpc::detail::net::ConnectionPtr new_conn);

    void OnConnect(const errcode&,RpcSession*);

private:
    // SessionID m_session; // session
    RpcSession* m_session;
    // yrpc::coroutine::poller::Epoller* scheduler_;   //协程调度器
    yrpc::detail::net::YAddress servaddr_;         //  服务端地址
    // yrpc::detail::net::Connector connector_;       //  
    yrpc::util::buffer::Buffer buffer_;             //  协议流
    std::shared_ptr<yrpc::rpc::detail::RpcClientSession> session_;
    std::thread* thread_;                           //thread
    std::mutex lock_;
    std::atomic_bool close_;
    static const char InitLogName[];


};

const char RpcClient::InitLogName[] = "client.log";




//////////////////////////////////////////////////////////
////////                                //////////////////
////////     template definition        //////////////////
////////                                //////////////////
//////////////////////////////////////////////////////////


// template<class Req,class Rsp>
// RpcClient<Req,Rsp>::RpcClient(std::string ip,int port,std::string logpath=InitLogName,int stack_size,int maxqueue)
// {
//     assert(scheduler_!=nullptr);
//     thread_ = new std::thread(RpcClient::run,this);
//     assert(thread_);
//     connector_.setOnConnect([this](yrpc::detail::net::ConnectionPtr conn,void*){NewConnection(conn);});
// }

// template<class Req,class Rsp>
// RpcClient<Req,Rsp>::RpcClient(yrpc::detail::net::YAddress servaddr_,std::string logpath=InitLogName,int stack_size,int maxqueue)
// {
//     assert(scheduler_!=nullptr);
//     thread_ = new std::thread(RpcClient::run,this);
//     assert(thread_);
// }









// template <class Req, class Rsp>
// bool RpcClient::async_call_long(std::string name,ReqPtr<Req>send, std::function<void(Rsp&)> f)
// {
//     static detail::RpcSession<Req,Rsp>* future = new detail::RpcSession<Req,Rsp>(scheduler_,name,servaddr_);

//     {
//         std::lock_guard<std::mutex> lock(lock_);
//         scheduler_->AddTask([this,&send,f](void* args){
//             detail::RpcSession<Req,Rsp>* future = (detail::RpcSession<Req,Rsp>*)args;
//             RspPtr<Rsp> ret = std::make_shared<Rsp>();
//             uint64_t n=-1;
//             while((n = future->Async_Call(send,f)) <= 0)
//                 yrpc::socket::YRSleep(this->scheduler_,1);
//         },future);
//     }
//     return true;
// }


// template<class Req,class Rsp>
// RpcClient::ResultPtr<Req,Rsp> RpcClient::result_long(std::string name,ReqPtr<Req> send)
// {
//     ResultPtr<Req,Rsp> result_long = std::make_shared<Result<Req,Rsp>>();
//     static detail::RpcSession<Req,Rsp>* connmanager = new detail::RpcSession<Req,Rsp>(scheduler_,name,servaddr_);    // 创建session

//     {
//         std::lock_guard<std::mutex> lock(lock_);
            // 添加一个协程，这个协程就是注册call请求到Session,session会自动发送数据
//         scheduler_->AddTask([&result_long,this,&send](void*args){
//             detail::RpcSession<Req,Rsp>* future = (detail::RpcSession<Req,Rsp>*)args;
//             RspPtr<Rsp> ret = nullptr;
//             std::shared_ptr<CallObj<Req,Rsp>> n;
//             while( (n=future->Call(send)) == nullptr)
//                 yrpc::socket::YRSleep(this->scheduler_,1);
//             result_long->SetCallObj(n);
//         },connmanager);
//     }
//     return result_long;
// }



}

