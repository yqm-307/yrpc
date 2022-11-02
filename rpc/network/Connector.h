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
 * 
*/
class Connector
{
public:
    Connector(yrpc::coroutine::poller::Epoller* loop);
    ~Connector();

    void AsyncConnect(RoutineSocket* ,YAddress servaddr,OnConnectHandle conn);

    static RoutineSocket* CreateSocket();
    static void DestorySocket(RoutineSocket*);

protected:
    void onConnect(RoutineSocket* servfd_,const YAddress& servaddr_,OnConnectHandle onconnect_);
private:
    yrpc::coroutine::poller::Epoller* scheduler_;

};

}