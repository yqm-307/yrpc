#pragma once
#include "../network/all.h"
#include "../shared/all.h"


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
*/
class Channel
{
public:
    typedef yrpc::util::buffer::Buffer                      Buffer;
    typedef yrpc::detail::shared::errorcode                 errorcode;
    typedef std::function<void(const errorcode&,Buffer&)>                    RecvCallback;
    typedef std::function<void(const errorcode&,size_t)>    SendCallback;
    typedef std::function<void(const errorcode&)>           CloseCallback;
    typedef std::function<void(const errorcode&)>           ErrorCallback;

public:
    Channel();
    ~Channel();


public:
    /**
     * 接收数据实现类似于一个dispatch的效果，解析并转发给服务提供或者服务调用返回，因为本
    */
    void SetRecvCallback(RecvCallback cb)  
    { m_recvcallback = cb; }

    void SetSendCallback(SendCallback cb)
    { m_sendcallback = cb; }

    void SetErrorCallback(ErrorCallback cb)
    { m_errorcallback = cb; }

    void SetCloseCallback(CloseCallback cb)
    { m_closecallback = cb; }

private:
    yrpc::detail::net::Connection       m_conn;     // 连接


    RecvCallback        m_recvcallback;
    SendCallback        m_sendcallback;
    CloseCallback       m_closecallback;
    ErrorCallback       m_errorcallback;

};



}