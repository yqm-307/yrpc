#pragma once
#include "Channel.h"
#include "../Util/Locker.h"
#include "../Util/Statistics.h"
#include "../protocol/all.h"
#include "../network/SessionBuffer.h"
#include "CallObjFactory.h"
#include "Define.h"
#include <bbt/poolutil/IDPool.hpp>
#include <bbt/uuid/Uuid.hpp>

namespace yrpc::rpc::detail
{

/**
 * @brief 双向连接，一个session可以做服务
 * 1、感觉数据保存这里最好，如果在manager，粒度太粗，manager不好管理
 * 2、心跳还是放在manager，统一管理
 * 3、session建立还是需要从manager的main epoll里面（只负责监听的主epoll）创建
 * 4、从属epoll就是 m 个连接在一个epoll里面，就是协程了。
 * 5、buffer 就设置在Session里面吧
 */
class RpcSession : public std::enable_shared_from_this<RpcSession>
{
    friend SessionManager;
    typedef yrpc::util::buffer::Buffer                     Buffer;
    
    struct session_detail_protocol
    {
        enum type : int8_t{
            done = 0,
            req = 1,
            rsp = 2,
        };

        Buffer data{""};
        type t{type::done};
    };
    typedef yrpc::detail::shared::errorcode     errorcode;
    typedef Channel::SPtr                       ChannelPtr;
    typedef yrpc::detail::net::Connection::SPtr ConnPtr;
    typedef yrpc::util::lock::Mutex             Mutex;
    typedef yrpc::coroutine::poller::Epoller    Epoller;
    typedef yrpc::detail::net::SessionBuffer    SessionBuffer;
    typedef std::map<Protocol_PckIdType,detail::CallObj::Ptr> CallObjMap;      

    
    template<class T>
    using lock_guard = yrpc::util::lock::lock_guard<T>;
public:
    typedef std::shared_ptr<RpcSession>         SessionPtr;
    typedef session_detail_protocol             Protocol;
    typedef std::queue<Protocol>                PckQueue;
    typedef std::function<void(const yrpc::detail::shared::errorcode&, SessionPtr)>   SessionCloseCallback;
    typedef std::function<void(const yrpc::detail::shared::errorcode&, SessionPtr)>   SessionTimeOutCallback;
    typedef std::function<void(const yrpc::detail::shared::errorcode&, SessionPtr)>   SessionHandShakeTimeOutCallback;


public:
    RpcSession(ChannelPtr channel);
    ~RpcSession();

    /* 发送数据 */
    size_t Append(const std::string_view pck);
    /* 发送数据 */
    size_t Append(const Buffer& bytearray);

    //////////////////////
    ///// 连接控制 ///////
    //////////////////////
    /* 连接是否已经关闭 */
    bool IsClosed();
    /* 关闭连接 */
    void Close();

    // void ForceClose();

    /* 创建一个Session */
    static SessionPtr Create(ChannelPtr channel)
    { return std::make_shared<RpcSession>(channel); }

    /* 获取对端地址 */
    const Address& GetPeerAddress();
        
    /* 不要在多线程环境调用。线程不安全 */
    void SetCloseFunc(SessionCloseCallback f)
    { m_closecb = f; } 
    /* 不要在多线程环境调用。线程不安全 */ 
    void SetTimeOutFunc(SessionTimeOutCallback f)
    { m_timeoutcallback = f; }

    /* 初始化所有回调 */
    void UpdataAllCallbackAndRunInEvloop();
    
    /* 发送一个CallObj。线程安全 */
    int SendACallObj(detail::CallObj::Ptr obj);

    void SetPeerUuid(bbt::uuid::UuidBase::Ptr uuid);
    bbt::uuid::UuidBase::Ptr GetPeerUuid();
private:
    /* 添加一个obj到objmap中，成功返回1，失败返回-1。线程安全 */
    int CallObj_AddObj(detail::CallObj::Ptr obj);
    /* 从objmap中删除一个obj，成功返回1，失败返回-1。线程安全 */
    int CallObj_DelObj(Protocol_PckIdType id);
    /* 设置调用结果 */
    int CallObj_CallResult(Buffer&& buf);
    /* 连接活跃了，更新超时 */
    void UpdateTimeout();
    
    // 获取一个协议包，失败返回的protocol 字节数为0。线程安全
    Protocol GetAPacket();
    // 获取当前所有协议包，失败返回一个空的queue,尽量使用GetAllPacket。线程安全
    PckQueue GetAllPacket();
    // 当前是否有协议，线程安全
    bool HasPacket();

    /**
     * @brief 将当前数据进行分解，并放在c2s、s2c队列中
     */
    std::vector<Protocol> GetProtocolsFromInput();

    void HandleProtocol(const std::vector<Protocol>& protocols);

    void AddPacket(const Protocol& pck);

    void InitFunc();
    ////////////////////////////////////////////////////////////////////////
    //////// 回调操作
    ////////////////////////////////////////////////////////////////////////
    // thread safe
    void RecvFunc(const errorcode&,Buffer&);
    void SendFunc(const errorcode&,size_t);
    void OnClose(const errorcode&, Channel::SPtr);
    void TimeOut(Socket* socket);
public:
    ////////////////////////////////////////////////////////////////////////
    //////// 握手相关操作
    ////////////////////////////////////////////////////////////////////////
    /* 开启握手定时器并设置超时回调 */
    void StartHandShakeTimer(const SessionHandShakeTimeOutCallback& f, int timeout_ms);
    /* 关闭握手定时器 */
    void StopHandShakeTimer();

private:
    void Dispatch(Buffer&& buf, SessionPtr sess);
    // void CallResult(Buffer&& buf, SessionPtr sess);
private:
    /// 当前所在的eventloop
    Epoller*        m_current_loop{nullptr};

    /// input 协议队列
    SessionBuffer   m_input_buffer;         // input buffer 
    Mutex           m_input_mutex;
    PckQueue        m_pck_queue;
    Mutex           m_mutex_pck;
    /// 统计，debug使用
#ifdef YRPC_DEBUG
    yrpc::util::statistics::ByteRecord m_byterecord;
#endif

    /// 很重要的双向信道
    ChannelPtr      m_channel;      // io 信道
    char*           m_remain{nullptr};       // 不完整的包

    std::atomic_bool    m_can_used; // session是否可用
    yrpc::util::clock::Timestamp<yrpc::util::clock::ms> 
                    m_last_active_time; // 最后活跃时间
    CallObjMap      m_call_map;         /* 本地向远端调用事件的集合 */
    Mutex           m_mutex_call_map;

    SessionCloseCallback            m_closecb{nullptr};
    SessionTimeOutCallback          m_timeoutcallback{nullptr};
    SessionHandShakeTimeOutCallback m_handshake_callback{nullptr};
    std::atomic_bool                m_handshake_time_isstop;

    static bbt::pool_util::IDPool<int,true>
                                    g_sessionid_mgr;    /* session id 管理 */
    bbt::uuid::UuidBase::Ptr        m_peer_node_uuid;     /* 对端节点uuid */
    // bbt::uuid::UuidBase::Ptr        m_session_id;    // 目前看来不需要唯一标识 sid
};
}