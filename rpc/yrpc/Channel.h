#pragma once
#include "../network/all.h"
#include "../shared/all.h"
#include "../Util/Locker.h"

namespace yrpc::rpc::detail
{

/**
 * 思考一个问题，如果一个进程既作为服务提供方，又作为服务调用方怎么办？
 * 
 * 1、首先是否需要建立两条tcp连接？
 *      不需要，可以有一个过滤器做过滤分发即可
 * 2、为什么要Channel？
 *      其实就是对connection的再次封装，因为 network 部分有点丑陋。希望可以通过 channel 这一层封装屏蔽掉底层接口。
 *      顺便改变一下 session 的功能，只去实现网络上层逻辑即可，就是只需要关注 rpc 的其他功能。
 *      当然，感觉有点奇怪，为什么越封装越多了。
 * 
 * 3、Channel 的功能
 *      希望实现过滤器层面的协议分发，在TCP之上，应用层之下。并且提供包括健康检测、超时、重连等功能。
 * 
 * 4、Channel 是无锁的,不保证线程安全,需要上层的封装去保证Channel Send、Recv是线程安全的
 *
*/
class Channel
{
public:
    typedef yrpc::util::buffer::Buffer                      Buffer;
    typedef yrpc::detail::shared::errorcode                 errorcode;
    typedef yrpc::detail::net::ConnectionPtr                ConnPtr;
    typedef yrpc::coroutine::poller::Epoller                Epoller;
    typedef std::function<void(const errorcode&,Buffer&,const ConnPtr)>   RecvCallback;
    typedef std::function<void(const errorcode&,size_t,const ConnPtr)>    SendCallback;
    typedef std::function<void(const errorcode&,const ConnPtr)>           CloseCallback;
    typedef std::function<void(const errorcode&,const ConnPtr)>           ErrorCallback;
    typedef std::function<void()>                                         TimeOutCallback;
    typedef std::shared_ptr<Channel>                        ChannelPtr;
    typedef yrpc::util::lock::Mutex                         Mutex;
    
    template<class T>
    using lock_guard = yrpc::util::lock::lock_guard<T>;
    enum ChannelStatus : int32_t
    {
        Done = 1,       // 未初始化
        Reading = 1<<1,    // 正在读
        Writing = 1<<2        // 正在写
    };
public:



    Channel();
    Channel(ConnPtr new_conn,Epoller*);
    virtual ~Channel();


    ////////////////////////
    ////// 连接接口 ////////
    ////////////////////////

    /**
     * @brief 主动优雅关闭连接(优雅关闭,并非 linux socket 层面的 so_linger 设置,是指将buffer中数据完全发送完毕)
     *  调用该接口后，不能接收和发送数据
     */
    void Close();
    
    /* check connection is closed */
    bool IsClosed();
    
    /* check peer is alive */
    bool IsAlive();

    ////////////////////////
    //////  IO接口  ////////
    ////////////////////////
    /* send data to peer (thread safe) */
    size_t Send(const Buffer& data);

    /* send len byte to peer (thread safe)*/
    size_t Send(const char* data,size_t len);
    

    const ConnPtr GetConnInfo()
    { return m_conn; }




public:
    void SetRecvCallback(RecvCallback cb)  
    { m_recvcallback = cb; }
    void SetSendCallback(SendCallback cb)
    { m_sendcallback = cb; }
    void SetErrorCallback(ErrorCallback cb)
    { m_errorcallback = cb; }
    void SetCloseCallback(CloseCallback cb)
    { m_closecallback = cb; }
    void SetTimeOutCallback(TimeOutCallback cb)
    { m_timeoutcallback = cb; }
    static ChannelPtr Create(ConnPtr conn,Epoller* ep);


private:
    void CloseInitFunc(const errorcode&);
    void SendInitFunc(const errorcode&,size_t);
    void ErrorInitFunc(const errorcode&);
    // static void RecvInitFunc(const errorcode&,Buffer&,const ConnPtr);    // 应该要求上层封装提供
private:
    void InitFunc();

    /**
     * @brief 协程
     * 
     * @param data  待发送字节流 
     * @param len   待发送字节流长度
     * @return void   以Callback的形式完成发送后回调
     */
    void EpollerSend(const char* data,size_t len);
private:

    Epoller*        m_eventloop;
    ConnPtr         m_conn;     // channel hold conn
    volatile int    m_status;   // 信道应该带有io状态，正在读、正在写、空闲
    bool            m_is_closed{false};
    
    Buffer          m_buffer;    // 单缓冲
    Mutex           m_mutex_buff;

    RecvCallback        m_recvcallback;
    SendCallback        m_sendcallback;
    CloseCallback       m_closecallback;
    ErrorCallback       m_errorcallback;
    TimeOutCallback     m_timeoutcallback;
};



}