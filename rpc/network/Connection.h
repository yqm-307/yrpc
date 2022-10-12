/**
 * @file connection.h
 * @author your name (you@domain.com)
 * @brief tcpconnection 负责socket前数据层原始比特流收发
 * @version 0.1
 * @date 2022-09-16
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#pragma once
#include "Callback.h"
#include "IPAddress.h"
#include "../YRoutine/Hook.h"


namespace yrpc::detail::net
{
typedef yrpc::coroutine::poller::RoutineSocket RoutineSocket;


enum CONN_STATUS
{
    connected=0,    //已经建立连接
    connecting=1,   //正在建立连接
    disconnect=2    //断开连接
};

class Connection:std::enable_shared_from_this<Connection>
{
    typedef std::shared_ptr<Connection> ConnectionPtr;
    typedef yrpc::util::buffer::Buffer Buffer;
public:

    Connection(yrpc::coroutine::poller::Epoller* scheduler,RoutineSocket* sockfd,const YAddress& cli);
    ~Connection();

    ConnectionPtr GetPtr()
    { return this->shared_from_this(); }
    
    /**
     * @brief 发送数据
     * 
     * @param data 数据
     * @param len 字节数
     * @return size_t 发送成功字节，正常发送会yield直到发送完毕，但是如果socket异常可能发送失败
     */
    size_t send(const char* data,size_t len);

    size_t send(const Buffer& data);
    
    /**
     * @brief 接受数据到接受缓冲区 
     * 
     * @param buffer 接收缓冲区，应确保为空
     * @param buflen 缓冲区最大长度
     * @return size_t 成功写入buffer字节数；如果返回0，对端关闭；如果返回-1，读取错误。
     */
    size_t recv(char* buffer,size_t buflen);

    size_t recv(Buffer& data);

    /*关闭连接，但是等待本次传输完成*/
    void Close();
    /*强制关闭连接，释放资源*/
    void ForceClose();
    
    void setOnRecvCallback(OnRecvHandle cb)
    { onrecv_ = cb; }
    void setOnCloseCallback(ConnCloseHandle cb)
    { closecb_ = cb; }
    void update()
    { schedule_->AddTask([this](void*){recvhandler();},nullptr); }

    /*当前套接字是否超时，默认超时时间5s*/
    bool IsTimeOut()
    { return socket_->eventtype_ == yrpc::coroutine::poller::EpollREvent_Timeout; }
    /*conn关闭，返回true*/
    bool IsClosed()
    { return conn_status_ == disconnect;}

    std::string StrIPPort()
    { return cliaddr_.GetIPPort(); }
protected:
    //是否已经建立连接
    bool is_connected()
    {return conn_status_ == connected;}

    void runhandle();

    void initclosehandler(ConnectionPtr conn)
    {
        DEBUG("client :%s , info: connection close!",conn->StrIPPort().c_str());
    }

    void recvhandler();

protected:
    //todo outputbuffer ，不会让服务的写操作阻塞。但是rpc对于这个有要求吗？毕竟服务完成到返回都可以算作整体，而且有协程，处理完该发不出去还是发不出去
    yrpc::coroutine::poller::RoutineSocket* socket_;
    yrpc::coroutine::poller::Epoller* schedule_;    //由拥有者赋予
    
    CONN_STATUS conn_status_;
    YAddress cliaddr_;

    bool Reading_;
    bool Writing_; 

    OnRecvHandle    onrecv_;
    ConnCloseHandle closecb_;

};


}
