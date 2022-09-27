#pragma once
#include "../network/TcpServer.h"
#include "../msg/servicemap.h"
#include "./Service.h"
#include "../Util/ThreadPool.h"



namespace yrpc::rpc::detail
{

class ServerSingle final:yrpc::util::noncopyable::noncopyable
{
public:
    typedef std::function<void()> OneConnTask;
    /*创建监听套接字，启动*/
    ServerSingle(yrpc::coroutine::poller::Epoller* sche, int port,int socket_timeout_ms,int connect_timeout_ms,int stack_size);
    ~ServerSingle();

    void setOnConnect(yrpc::detail::ynet::OnConnectHandle func);

    //服务器退出前循环
    void run();

    //服务器关闭
    void close();

private:
    void OnSendHandle(const yrpc::detail::ynet::ConnectionPtr&conn,void*);
    void OnConnHandle(const yrpc::detail::ynet::ConnectionPtr&conn,void*);
private:
    int socket_timeout_ms_;
    int connect_timeout_ms_;
    yrpc::coroutine::poller::Epoller* scheduler_;
    yrpc::detail::ynet::Acceptor acceptor_;
    yrpc::detail::ynet::OnConnectHandle onconnectcb_;
    yrpc::detail::ynet::OnRecvHandle onrecvcb_;
    yrpc::rpc::detail::CallCenter caller_;
    volatile bool closed_;
};

}