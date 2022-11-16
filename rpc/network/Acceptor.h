#pragma once
#include "Connection.h"
#include "../Util/Type.h"
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
    Acceptor(yrpc::coroutine::poller::Epoller* loop,int port,int socket_timeout_ms = SOCKET_TIME_OUT_MS,int connect_timeout_ms = SOCKET_CONN_TIME_MS);
    ~Acceptor();


    /**
     * @brief 开启一个监听的循环事件，注册非阻塞异步任务到event loop 中。
     * 
     * @return int 如果返回值大于等于0，注册成功；如果返回值是-1，说明循环正在运行中；如果返回值为-2，说明尚未设置回调函数；
     */
    int StartListen();


    /**
     * @brief 停止监听
     */
    void Close(){ close_.store(true); }

    
    /**
     * @brief 上层持有者,通过注册该回调来开始运行
     * 
     * @param conn 
     * @param args 
     */
    template<typename Func,typename = yrpc::util::type::TypeIs<Func,OnConnectHandle>>
    void setOnConnect(Func&& conn,void*args=nullptr)
    { onconnection_ = conn; args_ = args;}
protected:
    /* 实际上处理连接事件的函数 */
    void listen_once();
    void Init();
    void ListenRunInLoop();
    void CreateListenSocket();
    void ReleaseListenSocket();
private:
    yrpc::coroutine::poller::Epoller* scheduler_;
    RoutineSocket* listenfd_;
    int port_;
    int fd_;
    std::atomic_bool close_;
    OnConnectHandle onconnection_;  //handle 去解析rpc request ，调用请求的服务
    void* args_;

    int connect_timeout_ms_;
    int socket_timeout_ms_;
};  

}