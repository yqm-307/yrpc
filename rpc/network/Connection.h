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
#include "Defines.h"
#include "IPAddress.h"
#include "../YRoutine/Hook.h"


namespace yrpc::detail::net
{
typedef yrpc::coroutine::poller::RoutineSocket Socket;


enum CONN_STATUS : int32_t
{
    connected=0,    //已经建立连接
    connecting=1,   //正在建立连接
    disconnect=2    //断开连接
};

class Connection:std::enable_shared_from_this<Connection>
{
public:
    typedef std::shared_ptr<Connection> ConnectionPtr;
    typedef std::function<void()>       OnTimeoutCallback;
    typedef yrpc::util::buffer::Buffer Buffer;
public:

    Connection(yrpc::coroutine::poller::Epoller* scheduler,Socket* sockfd,const YAddress& cli);
    ~Connection();

    // 获取 shared_form_this
    ConnectionPtr GetPtr();
    
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


    /**
     * @brief   关闭本次连接，但是等待缓冲区数据发送完毕
     */
    void Close();


    /**
     * @brief   强制关闭 socket fd, 释放资源
     */
    void ForceClose();
    

    /**
     * @param cb std::function<void(const errorcode&,yrpc::util::buffer::Buffer&)>
     */
    void setOnRecvCallback(OnRecvHandle cb);
    /**
     * @param cb std::function<void(const errorcode&,const ConnectionPtr&)>
     */
    void setOnCloseCallback(ConnCloseHandle cb);

    void setOnTimeoutCallback(OnTimeoutCallback cb);


    // 设置完回调一定要更新
    void update();

    /**
     * @brief 
     * 
     * @return true 
     * @return false 
     */
    bool IsTimeOut();


    /**
     * @brief 判断连接是否端开
     * 
     * @return true 
     * @return false 
     */
    bool IsClosed();
    
    /**
     * @brief 获取对端地址
     * 
     * @return const YAddress&  
     */
    const YAddress& GetPeerAddress() const;
    
    
    
    /**
     * @brief 获取对端IP Port 字符串形式
     * 
     * @return std::string 
     */
    std::string StrIPPort();
protected:
    //是否已经建立连接
    bool is_connected()
    {return m_conn_status == connected;}

    void runhandle();

    void initclosehandler(ConnectionPtr conn)
    { DEBUG("client :%s , info: connection close!",conn->StrIPPort().c_str()); }

    void recvhandler();

    void TimeOut(Socket* socket);
protected:
    //todo outputbuffer ，不会让服务的写操作阻塞。但是rpc对于这个有要求吗？毕竟服务完成到返回都可以算作整体，而且有协程，处理完该发不出去还是发不出去
    Socket*          m_socket;    
    yrpc::coroutine::poller::Epoller* m_schedule;    //由拥有者赋予
    
    CONN_STATUS             m_conn_status;
    YAddress                m_cliaddr;

    bool                    m_Reading;
    bool                    m_Writing; 

    OnRecvHandle            m_onrecv;
    ConnCloseHandle         m_closecb;   // 
    OnTimeoutCallback       m_timeoutcb;  // 超时回调通知
};


}
