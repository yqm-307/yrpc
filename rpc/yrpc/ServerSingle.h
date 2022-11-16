// #pragma once
// #include "../network/TcpServer.h"
// #include "../msg/servicemap.h"
// #include "./Service.h"
// #include "../Util/ThreadPool.h"



// namespace yrpc::rpc::detail
// {

// class ServerSingle final:yrpc::util::noncopyable::noncopyable
// {
// typedef std::function<void()> WorkFunc;
// public:

//     typedef std::function<void()> OneConnTask;
//     /*创建监听套接字，启动*/
//     ServerSingle(yrpc::coroutine::poller::Epoller* sche, int port,int socket_timeout_ms,int connect_timeout_ms,yrpc::util::threadpool::ThreadPool<WorkFunc>* threadpool=nullptr,int stack_size=64*1024);
//     ~ServerSingle();

//     void setOnConnect(yrpc::detail::net::OnConnectHandle func);

//     //服务器退出前循环
//     void run();



//     //服务器关闭
//     void close();

// private:
//     void OnSendHandle(const yrpc::detail::net::ConnectionPtr&conn,void*);
//     void OnConnHandle(const yrpc::detail::net::ConnectionPtr&conn,void*);
// private:
//     int socket_timeout_ms_;
//     int connect_timeout_ms_;
//     yrpc::coroutine::poller::Epoller* scheduler_;
//     yrpc::util::threadpool::ThreadPool<WorkFunc>* t_pool_ = nullptr; 

//     yrpc::detail::net::Acceptor acceptor_;
//     yrpc::detail::net::OnConnectHandle onconnectcb_;
//     yrpc::detail::net::OnRecvHandle onrecvcb_;
//     yrpc::rpc::detail::CallCenter caller_;
//     volatile bool closed_;
// };

// }