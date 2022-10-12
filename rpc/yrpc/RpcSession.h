// /**
//  * @file Future.h
//  * @author your name (you@domain.com)
//  * @brief 异步任务封装
//  * @version 0.1
//  * @date 2022-06-20
//  * 
//  * @copyright Copyright (c) 2022
//  * 
//  */
// #pragma once
// #include <memory>
// #include <functional>
// #include <condition_variable>
// #include "../YRoutine/Hook.h"
// #include "../protocol/ErrCode.h"
// #include "../network/Connector.h"
// #include "../Util/IDGenerate.h"
// #include "../network/SessionBuffer.h"
// // #include <YqmUtil/timer/interval.hpp>


// enum CALLTYPE
// {
//     CALL_NOT_START=0,   //调用为开始
//     CALL_IS_WAITTING,   //调用中，等待结果
//     CALL_IS_END,        //调用结束 
//     CALL_IS_FATAL       //失败，但是失败情况没有设置
// };


// template<class SendType,class RecvType>
// class CallObj
// {
// public:
//     typedef std::shared_ptr<RecvType> RecvPtr;
//     typedef std::shared_ptr<SendType> SendPtr;
//     typedef std::function<void(RecvType&)> AsyncFunc;
//     CallObj(yrpc::coroutine::poller::Epoller*sche,SendPtr send)
//         :send_(send),recv_(nullptr)
//     {
//         cond_.Init(sche,-1);
//     }
//     void Init(yrpc::coroutine::poller::Epoller*sche)
//     {
//         cond_.Init(sche,-1);
//         calltype_ = CALL_NOT_START;
//         send_=nullptr;
//         recv_=nullptr;
//     }
//     CallObj()=default;
//     ~CallObj(){}
//     void SetFunc(AsyncFunc func)
//     { async_func_ = func; }
//     void Async_Call()
//     { async_func_(*recv_); }
//     bool is_async_call()
//     { return async_func_ != nullptr;}
//     void Wait()
//     { cond_.Wait(); }
//     void Notify()
//     { cond_.Notify();}
//     void SetSend(SendPtr type)
//     { send_ = type;}
//     void SetRecv(RecvPtr type)
//     { recv_ = type; }
//     RecvPtr GetRecv()
//     { return recv_; }
//     SendPtr GetSend()
//     { return send_; }
//     CALLTYPE Type()
//     { return calltype_;}
//     void SetType(CALLTYPE type)
//     { 
//         calltype_ = type;
//         {
//             yrpc::util::lock::lock_guard<yrpc::util::lock::Spinlock> lock(lock_);
//             is_done_ = true;
//         }
//     }
//     bool Is_Done()
//     { return is_done_;}

//     void SetErrCode(int err)
//     { err_=err; }

//     int GetErrCode()
//     { return err_; }
// private:
//     yrpc::socket::Epoll_Cond_t cond_;
//     AsyncFunc async_func_{nullptr}; 
//     SendPtr send_;
//     RecvPtr recv_;
//     CALLTYPE calltype_{CALL_NOT_START};

//     bool is_done_{false};
//     yrpc::util::lock::Spinlock lock_;
//     std::atomic_int err_;
// };




// namespace yrpc::rpc::detail
// {
// #define YRPC_FUTURE_MAX_CALL 256

// typedef std::function<void()> FutureCallback;



// /**
//  * @brief rpc会话。内部有管理连接管理，SendProtol、RecvHandler、HeartBeat
//  * 
//  * @tparam SendType 
//  * @tparam RecvType 
//  */
// template<class SendType,class RecvType>
// class RpcSession
// {
// public:
//     typedef std::shared_ptr<RecvType> RecvPtr;
//     typedef std::shared_ptr<SendType> SendPtr;
//     typedef std::shared_ptr<CallObj<SendType,RecvType>> CallPtr; 

//     RpcSession(yrpc::coroutine::poller::Epoller* sche,std::string name,yrpc::detail::net::YAddress addr,int timeout = -1);
//     ~RpcSession(){}
    

//     bool Is_Connected()
//     {return connected_;}

//     void Connect()
//     { connector_.connect(); }

//     /**
//      * @brief 客户端或者服务端都可以通过session关闭会话
//      * 
//      * @return int 
//      */
//     int Close()
//     { return connected_;}

//     /**
//      * @brief 注册同步调用
//      * 
//      * @param sendpkg 
//      * @return uint64_t 
//      */
//     CallPtr Call(SendPtr sendpkg);

//     /**
//      * @brief 注册异步调用
//      * 
//      * @param ptr 
//      * @param f 
//      * @return uint64_t 
//      */
//     uint64_t Async_Call(SendPtr ptr,std::function<void(RecvType&)> f);

//     /**
//      * @brief 发送协议给对端
//      * 
//      * @param SendPtr ptr 将要发送的数据
//      * @return int >0 表示成功
//      */
//     int SendProtocol(SendPtr ptr);


// private:

//     /**
//      * @brief 一次连接建立时，处理函数
//      *  函数会以协程启动在RpcClient开启的线程中，其实是sche在RpcClient中run。所以其他地方注册协程也会在run所循环的线程中
//      * @param conn 连接
//      */
//     void OnConnHandle(const yrpc::detail::net::ConnectionPtr& conn);

//     //协程中处理本次连接的响应数据
//     void OnRecvHandle(const yrpc::detail::net::ConnectionPtr &conn);

//     //任务解析和消息回调,处理PkgManager数据
//     void MsgParseHandle();

//     //发送数据
//     void SendMsgHandle();


//     bool Encode(std::string &bytes, uint64_t call_id, const google::protobuf::Message *msg);

//     std::string DeCode(const char* data,size_t len);

//     void SetResult(CALLTYPE code,CallPtr ptr,uint64_t id=-1,RecvPtr result = nullptr);

//     void SetErrno(CALLTYPE code,CallPtr ptr,uint64_t id,int errcode);

// private:
//     typedef std::pair<uint64_t,CallPtr> CallSlot;
//     typedef std::map<uint64_t,CallPtr> CallMap; 
//     typedef std::map<uint64_t,CallPtr> ResultMap;
//     typedef std::queue<CallSlot> CallQueue;
    

//     yrpc::coroutine::poller::Epoller* scheduler_;
//     yrpc::socket::Epoll_Cond_t cond_t_;
//     yrpc::detail::net::Connector connector_;
//     bool connected_;
//     std::string name_;
//     yrpc::detail::net::SessionBuffer package_;    
    
//     yrpc::util::buffer::Buffer output_;     //Call 时解析并加入output buffer

    

//     char buf[4096];
//     char recvbuf[4096];



//     /*调用池不加锁，因为调用任务注册是在协程中执行*/
//     CallQueue callmap_;       //请求队列   
//     int max_size_{10240};
    
//     /*结果池需要加锁，因为需要跨线程共享数据*/
//     ResultMap resultmap_;   //结果池
//     std::mutex resultlock_; 


// };  



// #include "RpcSession_Definition.h"
// }
