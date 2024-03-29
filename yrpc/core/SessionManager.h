#pragma once
#include "ConnQueue.hpp"
#include "Define.h"
#include "yrpc/core/YRpc.h"
#include "bbt/uuid/Uuid.hpp"
#include "ChannelMgr.h"
#include <unordered_map>
namespace yrpc::rpc::detail
{

/**
 * @brief session 管理器，客户端最重要的管理模块
 * 1、实现了 TCP 连接复用
 */
class __YRPC_SessionManager : bbt::templateutil::noncopyable
{
public:
    typedef std::function<void(SessionPtr)>     OnSession;  
private:
    typedef bbt::uuid::UuidBase         Uuid;
    // typedef Uuid::Ptr UuidPtr;
    typedef std::function<uint32_t(const Uuid&)>    UuidHash;
    typedef std::function<bool(const Uuid&, const Uuid&)>   UuidEqual;
    typedef std::unordered_map<Uuid, SessionPtr, UuidHash, UuidEqual>    SessionMap;
    template<class T>
    using lock_guard = yrpc::util::lock::lock_guard<T>;
public:
    static __YRPC_SessionManager* GetInstance();

    /* 发起一个异步连接,成功后会调用回调 */
    void AsyncConnect(Address peer,OnSession onsession);
    /* 异步接收连接 */
    void StartListen(const Address& peer);
    /* 尝试获取Session，Session不存在或者正在连接中返回nullptr，线程安全 */
    SessionPtr TryGetSession(const Address& peer);
    /* 当前地址是否正在连接中 */
    bool IsConnecting(const Address& peer);
    void RegisterService();
private:
    __YRPC_SessionManager(int Nthread);
    ~__YRPC_SessionManager();

    ////////////////////////////////////////////////////////////////////////
    //////// 普通回调操作
    ////////////////////////////////////////////////////////////////////////
    // 运行在 sub loop 中的，只做io、协议解析
    void RunInSubLoop(Epoller*);
    /* 成功接收到连接 */
    void OnAccept(const errorcode &e, Channel::SPtr conn);
    /* 连接对端成功 */
    void OnConnect(const errorcode &e, Channel::SPtr conn, const Address& addr);
    /* 负载均衡策略 */
    Epoller* LoadBalancer();
    /* 子线程 */
    void SubLoop(int idx);
    /* 主线程 */
    void MainLoop();
    /* RpcSession 关闭时调用 */
    void OnSessionClose(const yrpc::detail::shared::errorcode& e, SessionPtr addr);
    /* RpcSession 超时时调用 */
    void OnSessionTimeOut(const yrpc::detail::shared::errorcode& e, SessionPtr addr);
    ////////////////////////////////////////////////////////////////////////
    //////// 其他
    ////////////////////////////////////////////////////////////////////////
    /* 初始化一个rpc session */
    SessionPtr InitRpcSession(Channel::SPtr chan);
    /* 主循环初始化时调用 */
    void OnMainLoopInit();
    /* 分配buffer，不该放这里 */
    void Dispatch(Buffer&&string, SessionPtr sess);

#pragma region // session map 操作集合, 不加锁

    // 向 m_session_map 中添加一个session, 失败返回 nullptr
    SessionPtr Append_SessionMap(Uuid uuid, SessionPtr newconn);
    // 从 m_session_map 中删除一个session, 失败返回 nullptr
    SessionPtr Delete_SessionMap(Uuid uuid);
    // 从 know_host 中获取一个 uuid, 失败返回 nullptr
    Uuid GetUuid(const Address& key);
    SessionID AddressToID(const Address&key);
    /* 添加一个新的Session 到半连接队列中 */
    SessionPtr Append_UnDoneMap(RpcSession::SPtr newconn);
    /* 从半连接队列中删除一个Session连接，失败返回 nullptr */
    std::pair<HandShakeData, bool> Delete_UnDoneMap(const Address& peer_addr);
    /* 从半连接队列中删除一个 TcpConn，失败返回 false */
    bool Delete_TcpUnDoneMap(const Address& peer_addr);
    SessionPtr GetSessionFromSessionMap(Uuid uuid);
#pragma endregion


    ////////////////////////////////////////////////////////////////////////
    //////// 握手
    ////////////////////////////////////////////////////////////////////////
    /* 接受握手请求，并响应 */
    MessagePtr Handler_HandShake(const MessagePtr, SessionPtr sess);
    /* 发送握手请求 */
    void StartHandShake(const yrpc::detail::shared::errorcode& e, SessionPtr sess);
    /* 处理握手响应 */
    void HandShakeRsp(MessagePtr, SessionPtr);
    /* 握手完成回调 */
    void OnHandShakeFinal(const yrpc::detail::shared::errorcode& e, SessionPtr sess);
    /* 握手超时, 对于调用方和被调用方都是一样的操作 */
    void OnHandShakeTimeOut(const yrpc::detail::shared::errorcode& e, SessionPtr sess);
private:
    Epoller*            m_main_loop;        // 只负责 listen 的 epoll
    Address             m_local_addr;       // 服务器本地地址（监听地址）
    const size_t        m_sub_loop_size;    // sub eventloop 数量
    std::vector<Epoller*>           m_sub_loop;// sub eventloop
    CountDownLatch      m_loop_latch;       // 
    std::atomic_int     m_balance;          // 新连接轮转负载，跨线程（需要atomic，还需要考虑 memory order）
    
    std::thread*        m_main_thread;      
    std::vector<std::thread*> m_sub_threads;   

    /////////////////////////////////
    SessionMap          m_session_map;      // 全连接 map <uuid, session>
    ConnQueue::Ptr      m_undone_conn_queue;// 半连接 queue 
    std::unordered_map<Address,Uuid>   
                        m_knownode_map;
    Mutex               m_mutex_session_map;
    /////////////////////////////////

    Uuid                m_local_node_id;

    std::vector<Uuid>   m_node_id_list;

    ChannelMgr::SPtr    m_channel_mgr;
};
}