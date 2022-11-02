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
// #include "./RpcClientSession.h"
#include "./SessionManager.h"
#include "./Define.h"
#include "CallObj.h"

namespace yrpc::rpc
{

/**
 * @brief RpcClient 是调用方最重要的类，通过该类才可以发起调用
 */
class RpcClient
{
    typedef detail::__YRPC_SessionManager                   SessionManager;
    typedef detail::__YRPC_SessionManager::SessionID        SessionID;
    typedef detail::RpcSession                              RpcSession;
    typedef yrpc::detail::shared::errorcode                 errcode;
    typedef yrpc::detail::net::YAddress                     Address;
    typedef std::shared_ptr<RpcSession>                     SessionPtr;
    typedef google::protobuf::Message                       Message;
    typedef std::map<uint64_t,CallObj::Ptr>                      CallObjMap;               
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
    RpcClient(std::string ip,int port);
    RpcClient(yrpc::detail::net::YAddress servaddr_);
    ~RpcClient();   //todo,退出不安全，在Client关闭时，应该先让所有请求都返回或者直接失败
    
    bool IsConnected();
    int AsyncCall();
    int SyncCall();



private:
    void OnConnect(SessionPtr newsession);

private:
    SessionPtr          m_session;      // 实现rpc操作
    Address             m_addr;
    CallObjMap          m_map;          // map
};





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

