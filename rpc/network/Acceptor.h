#pragma once
#include "Connection.h"
#include "../shared/all.h"


namespace yrpc::detail::net
{

class Acceptor
{
    typedef yrpc::detail::shared::errorcode errorcode;
public:
    /**
     * @brief Construct a new Acceptor object
     * 
     * @param loop 协程调度器
     * @param port 服务器端口号
     * @param socket_timeout_ms socket超时时间 
     * @param connect_timeout_ms 连接超时时间
     */
    Acceptor(yrpc::coroutine::poller::Epoller* loop,int port,int socket_timeout_ms,int connect_timeout_ms);
    ~Acceptor();


    //协程任务
    void listen();
    void close(){ close_.exchange(true); }
    void setOnConnect(OnConnectHandle&& conn,void*args=nullptr)
    { onconnection_ = conn; args_ = args;}
protected:
    void CreateListenSocket();
    void ReleaseListenSocket();
private:
    yrpc::coroutine::poller::Epoller* scheduler_;
    RoutineSocket* listenfd_;
    int fd_;
    std::atomic_bool close_;
    OnConnectHandle onconnection_;  //handle 去解析rpc request ，调用请求的服务
    void* args_;

    int connect_timeout_ms_;
    int socket_timeout_ms_;
};  

}