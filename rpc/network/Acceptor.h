#pragma once
#include "Connection.h"
#include "../shared/all.h"


namespace yrpc::detail::net
{

class Acceptor
{
    typedef yrpc::coroutine::poller::Epoller    Epoller;
    typedef std::function<Epoller*()>           LoadBalancer;
    typedef yrpc::detail::shared::errorcode     errorcode;
public:
    /**
     * @brief Construct a new Acceptor object
     * 
     * @param loop 协程调度器
     * @param port 服务器端口号
     * @param socket_timeout_ms socket超时时间 
     * @param connect_timeout_ms 连接超时时间 
     */
    Acceptor(Epoller* loop,int port,int socket_timeout_ms = SOCKET_TIME_OUT_MS,int connect_timeout_ms = SOCKET_CONN_TIME_MS);
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
     *  (ps:我们希望使用万能引用,因此需要使用模板,但是又想要限定类型,
     *  因此需要偏特化模板,所以使用 if_same_as )
     * @param onconn 新连接建立时回调
     * @param args  函数参数(保留，可能用到)
     */
    template<typename Func,if_same_as(Func,OnConnectHandle)>
    void setOnAccept(Func&& onconn,void*args=nullptr)
    { onconnection_ = onconn; args_ = args;}

    template<typename LBer,if_same_as(LBer,LoadBalancer)>
    void setLoadBalancer(const LBer& lber)
    { lber_ = lber; }
protected:
    /* 实际上处理连接事件的函数 */
    void ListenInEvloop();
    void Init();
    // void ListenRunInLoop();
    void CreateListenSocket();
    void ReleaseListenSocket();
private:
    Epoller*        scheduler_;   // listen所在的evloop
    LoadBalancer    lber_;   
    Socket*         listenfd_;
    int             port_;
    int             fd_;
    std::atomic_bool    close_;
    OnConnectHandle     onconnection_;  //handle 去解析rpc request ，调用请求的服务
    void*               args_;

    int                 connect_timeout_ms_;
    int                 socket_timeout_ms_;
};  

}