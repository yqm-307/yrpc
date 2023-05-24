#pragma once
#include "Channel.h"
#include "../Util/Locker.h"
#include "../Util/Statistics.h"
#include "../protocol/all.h"
#include "../network/SessionBuffer.h"
#include "CallObjFactory.h"
#include "Define.h"

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
    typedef Channel::Buffer                     Buffer;
    
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
    typedef Channel::errorcode                  errorcode;
    typedef Channel::ChannelPtr                 ChannelPtr;
    typedef Channel::ConnPtr                    ConnPtr;
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
    typedef std::function<void(
        const yrpc::detail::shared::errorcode&,
        const yrpc::detail::net::YAddress&)>   SessionCloseCallback;

    typedef std::function<void(Buffer&&,SessionPtr)>        DispatchCallback_Server;
    typedef std::function<void(Buffer&&,SessionPtr)>        DispatchCallback_Client;

public:
    RpcSession(ChannelPtr channel,Epoller* loop);
    ~RpcSession();



    //////////////////////
    ///// 协议控制 ///////
    //////////////////////
    
    // thread safe,获取一个协议包，失败返回的protocol 字节数为0
    Protocol GetAPacket();
    // thread safe,获取当前所有协议包，失败返回一个空的queue,尽量使用GetAllPacket
    PckQueue GetAllPacket();
    // thread safe,当前是否有协议
    bool HasPacket();
    // thread safe,向output追加数据
    size_t Append(const std::string_view pck);
    // thread safe,向output追加数据
    size_t Append(const Buffer& bytearray);

    // 将 pck 给 client
    void SetToClientCallback(DispatchCallback_Client cb)
    { m_stoclient = cb; }
    // 将 pck 给 server
    void SetToServerCallback(DispatchCallback_Server cb)
    { m_ctoserver = cb; }

    //////////////////////
    ///// 连接控制 ///////
    //////////////////////
    bool IsClosed();

    void Close();

    void ForceClose();

    static SessionPtr Create(ChannelPtr channel,Epoller* ep)
    { return std::make_shared<RpcSession>(channel,ep); }

    const Channel::Address& GetPeerAddress();
        
    void SetCloseFunc(SessionCloseCallback f)
    { m_closecb = f; } 
    /* 超时函数，连接真的空闲才会调用 */ 
    void SetTimeOutFunc(Channel::TimeOutCallback f)
    { m_timeoutcallback = f; }


    void UpdataAllCallbackAndRunInEvloop();
    
    /**/
    int SendACallObj(detail::CallObj::Ptr obj);
private:
    /* 添加一个obj到objmap中，成功返回1，失败返回-1 */
    int CallObj_AddObj(detail::CallObj::Ptr obj);
    /* 从objmap中删除一个obj，成功返回1，失败返回-1 */
    int CallObj_DelObj(detail::CallObj::Ptr obj);
    /* 连接活跃了，更新超时 */
    void SetActive();
    /*
    * 这里是这个类最核心的部分，重要的几个函数，和大概功能我列出来。
    * Input  从channel接收数据
    * Output 从channel发送数据
    * ProtocolMultiplexing  协议多路复用和分解，调用Getpck 相关函数触发
    * 
    */

    /**
     * @brief 将当前数据进行分解，并放在c2s、s2c队列中
     */
    std::vector<Protocol> GetProtocolsFromInput();

    void HandleProtocol(const std::vector<Protocol>& protocols);

    // thread unsafe,Session上行数据
    void Input(const char*,size_t);

    // thread unsafe,Session下行数据
    void Output(const char*,size_t);

    void AddPacket(const Protocol& pck);

    void InitFunc();
    // thread safe
    void RecvFunc(const errorcode&,Buffer&);
    void SendFunc(const errorcode&,size_t);
    void CloseFunc(const errorcode&);
    void TimeOut(Socket* socket);

    // server handler 尚未注册
    void NoneServerHandler();
    // client handler 尚未注册
    void NoneClientHandler();
private:
    /// 当前所在的eventloop
    Epoller*        m_current_loop{nullptr};
    Mutex           m_push_mutex;

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
    CallObjMap      m_call_map;

    SessionCloseCallback            m_closecb{nullptr};
    DispatchCallback_Client         m_stoclient{nullptr};
    DispatchCallback_Server         m_ctoserver{nullptr};
    Channel::TimeOutCallback        m_timeoutcallback{nullptr};
};


}