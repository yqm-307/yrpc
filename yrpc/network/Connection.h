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
#include <bbt/templateutil/BaseType.hpp>


namespace yrpc::detail::net
{
typedef yrpc::coroutine::poller::RoutineSocket Socket;


enum CONN_STATUS : int32_t
{
    connected=0,    //已经建立连接
    connecting=1,   //正在建立连接
    disconnect=2    //断开连接
};

class Connection : public std::enable_shared_from_this<Connection>, public bbt::templateutil::BaseType<Connection>
{
    // typedef std::shared_ptr<Connection> ConnectionPtr;
    typedef std::function<void(Socket*)>       OnTimeoutCallback;
    typedef yrpc::util::buffer::Buffer Buffer;
public:

    Connection(yrpc::coroutine::poller::Epoller* scheduler,Socket* sockfd,const YAddress& cli);
    ~Connection();    
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


    /* 关闭本次连接，但是等待缓冲区数据发送完毕 */
    void Close();

    /* 接收数据时回调 */
    void setOnRecvCallback(OnRecvHandle cb);
    /* socket 关闭时回调 */
    void setOnCloseCallback(ConnCloseHandle cb);
    /* socket 空闲超时回调 */
    void setOnTimeoutCallback(OnTimeoutCallback cb);


    // 启动 Connection 的接收函数
    void StartRecvFunc();
    /* 是否已经超时 */
    bool IsTimeOut();
    /* 连接是否关闭 */
    bool IsClosed();
    /* 获取对端地址 */
    const YAddress& GetPeerAddress() const;
    /* 获取对端ip port 的字符串 */
    std::string StrIPPort();
    /* 创建一个Connection，并返回智能指针 */
    static SPtr Create(yrpc::coroutine::poller::Epoller* scheduler,Socket* sockfd,const YAddress& cli);

    yrpc::coroutine::poller::Epoller* GetScheudler();
protected:
    //是否已经建立连接
    bool is_connected();
    // socket 层接收处理函数
    void RecvFunc();
    // socket 空闲超时
    void TimeOut(Socket* socket);
    // 检查调度器是否和当前运行中的调度器一致
    bool CheckScheduler();
protected:
    //todo outputbuffer ，不会让服务的写操作阻塞。但是rpc对于这个有要求吗？毕竟服务完成到返回都可以算作整体，而且有协程，处理完该发不出去还是发不出去
    Socket*          m_socket; // m_ssl_socket
    yrpc::coroutine::poller::Epoller* m_schedule;
    
    volatile CONN_STATUS             m_conn_status;
    YAddress                m_cliaddr;

    bool                    m_Reading;
    bool                    m_Writing; 

    OnRecvHandle            m_onrecv;
    ConnCloseHandle         m_closecb;   // 
    OnTimeoutCallback       m_timeoutcb;  // 超时回调通知

    int     m_init_buffer_size{4096};               // 初始化buffer大小      
    // input buffer
    yrpc::util::buffer::Buffer  m_input_buffer;     // 接收缓存
    yrpc::util::buffer::Buffer  m_output_buffer;    // 接收缓存
    
};


}
