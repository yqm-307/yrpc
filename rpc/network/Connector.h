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
class Connector : public std::enable_shared_from_this<Connector>
{
public:
    /**
     * @brief Construct a new Connector object
     * 
     * @param loop 
     */
    Connector(yrpc::coroutine::poller::Epoller* loop);
    ~Connector();


    /**
     * @brief 注册一个异步Connect，提供socket，服务端地址，回调。
     *  完成连接之后会回调通知
     * 
     * @param servaddr 服务端地址
     * @param onconn  连接成功时回调
     */
    template<typename T,if_same_as(T,OnConnectHandle)>
    void AsyncConnect(Socket* socket,YAddress servaddr,const T& onconn)
    {   

        scheduler_->AddTask([this,socket,servaddr,onconn](void*){
            this->onConnect(socket,servaddr,onconn);
        });
    }

    void SyncConnect(Socket* socket, YAddress servaddr);

    static Socket* CreateSocket();
    static void DestorySocket(Socket*);

protected:
    auto GetPtr()
    {
        return this->shared_from_this();
    }
    void onConnect(Socket* servfd_,const YAddress& servaddr_,const OnConnectHandle& onconnect_);
private:
    yrpc::coroutine::poller::Epoller* scheduler_;
};

}