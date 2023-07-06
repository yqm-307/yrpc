/**
 * @file Connector.h
 * @author your name (you@domain.com)
 * @brief Connector 需要补充心跳
 * @version 0.1
 * @date 2022-08-28
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#pragma once
#include "Connection.h"



namespace yrpc::detail::net
{


/***
 * Connector 纯函数实现。
 * 因为想要将 network 抽象出来，但是本人设计水平不高，目前想到的办法就是将 network 部分尽量做纯函数实现，
 * 类似ECS那样，网络库本身不保存数据，数据层在 channel 、 session
 * 
*/
class Connector : public std::enable_shared_from_this<Connector>, public bbt::templateutil::BaseType<Connector>
{
    typedef yrpc::coroutine::poller::Epoller    Epoller;
    typedef std::function<Epoller*()>           LoadBalancer;
    typedef std::function<void(const yrpc::detail::shared::errorcode&, Connection::SPtr, const yrpc::detail::net::YAddress&)> OnConnectCallback;    // 连接建立完成回调
public:
    /**
     * @brief Construct a new Connector object
     * 
     * @param loop 
     */
    Connector(yrpc::coroutine::poller::Epoller* loop);
    ~Connector();

    /* 向 servaddr 发起一个连接 */
    void AsyncConnect(YAddress servaddr)
    {   
        scheduler_->AddTask([this,servaddr](void*){
            this->Connect(servaddr);
        });
    }
    
    void setLoadBalancer(const LoadBalancer& lber)
    { m_lber = lber; }

    void SetOnConnectCallback(const OnConnectCallback& cb)
    { m_onconn = cb; }

    static Socket* CreateSocket();
    static void DestorySocket(Socket*);
    static SPtr Create(yrpc::coroutine::poller::Epoller* loop);
protected:
    auto GetPtr()
    {
        return this->shared_from_this();
    }
    void Connect(const YAddress& servaddr);
private:
    yrpc::coroutine::poller::Epoller* scheduler_;

    OnConnectCallback   m_onconn;
    
    LoadBalancer    m_lber;
};

}