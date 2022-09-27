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
#include "connection.h"



namespace yrpc::detail::ynet
{
class Connector
{
public:
    Connector(yrpc::coroutine::poller::Epoller* loop,YAddress servaddr,int socket_ms=5000,int conn_ms=3000);
    ~Connector();

    /**
     * @brief 调用connect，就会注册OnConnect到Epoller中
     */
    void connect();
    void setOnConnect(OnConnectHandle conn,void* arg = nullptr)
    { onconnect_ = conn; args_=arg;}
    void setClosed(OnCloseHandle close)
    { onclosed_ = close; }

protected:
    void onConnect();
    void CreateSocket();
    void ReleaseSocket();

private:
    yrpc::coroutine::poller::Epoller* scheduler_;
    RoutineSocket* servfd_;
    int fd_;
    YAddress servaddr_;

    OnConnectHandle onconnect_;
    OnCloseHandle onclosed_;

    void* args_;

    int socket_timeout_ms_;
    int connect_timeout_ms_;

};

}