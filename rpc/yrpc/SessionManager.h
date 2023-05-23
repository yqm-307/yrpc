#pragma once
#include "ConnQueue.hpp"
#include "Define.h"

namespace yrpc::rpc::detail
{

/**
 * @brief session 管理器，客户端最重要的管理模块
 * 1、实现了 TCP 连接复用
 */
class __YRPC_SessionManager : bbt::noncopyable
{
public:
    typedef std::shared_ptr<RpcSession>         SessionPtr;
    typedef std::function<void(SessionPtr)>     OnSession;
private:

    typedef std::unordered_map<SessionID,SessionPtr>   SessionMap;
    template<class T>
    using lock_guard = yrpc::util::lock::lock_guard<T>;
public:
    static __YRPC_SessionManager* GetInstance();  
    bool AsyncConnect(YAddress peer,OnSession onsession);
    void AsyncAccept(const YAddress& peer);
private:
    __YRPC_SessionManager(int Nthread);  
    ~__YRPC_SessionManager();

    // 运行在 main loop 中的，只做新连接的分发
    void RunInMainLoop();
    // 运行在 sub loop 中的，只做io、协议解析
    void RunInSubLoop(Epoller*);
    // 被连接后
    void OnAccept(const errorcode &e, ConnectionPtr conn);
    void OnConnect(const errorcode &e, ConnectionPtr conn);

    // 注意这是线程不安全的,获取一个新的session uid
    SessionID GetNewID()
    { return (++m_id_key); }
    Epoller* LoadBalancer();
private:
    // thread unsafe: 添加一个新的Session 到 SessionMap 中
    SessionPtr AddNewSession(Channel::ConnPtr newconn);
    // 此操作线程安全: 删除并释放 SessionMap 中一个Session 的资源。如果不存在，则返回false，否则返回true
    bool DelSession(const YAddress&);
    void Dispatch(Buffer&&string, SessionPtr sess);
    SessionID AddressToID(const YAddress&key);

private:
    Epoller*            m_main_loop;        // 只负责 listen 的 epoll
    Acceptor*           m_main_acceptor;    // listen 
    Connector           m_connector;        
    std::vector<Epoller*>           m_sub_loop;         // sub eventloop
    const size_t        m_sub_loop_size;    // sub eventloop 数量
    CountDownLatch      m_loop_latch;       // 
    std::atomic_int     m_balance;          // 新连接轮转负载，跨线程（需要atomic，还需要考虑 memory order）
    
    std::thread*        m_main_thread;      
    std::vector<std::thread*>       m_sub_threads;   

    /////////////////////////////////
    SessionMap                  m_session_map;      // 全连接 map
    ConnQueue::Ptr      m_undone_conn_queue;        // 半连接 queue 
    Mutex               m_mutex_session_map;
    /////////////////////////////////

    std::hash<std::string>  m_addrhash;     // 地址hash

    uint64_t            m_id_key{10000};  // session id 起名用的

    // main loop 控制
    std::atomic_bool    m_run;

    
    const int port;  
};
}