// #include "RpcClientSession.h"
#include "RpcSession.h"
#include "../network/all.h"
#include <vector>
#include <map>

namespace yrpc::rpc::detail
{

/**
 * @brief session 管理器，客户端最重要的管理模块
 * 1、实现了 TCP 连接复用
 */
class __YRPC_SessionManager : yrpc::util::noncopyable::noncopyable
{
public:
    typedef std::shared_ptr<RpcSession>         SessionPtr;
    typedef uint64_t SessionID;
    typedef std::function<void(SessionPtr)>    OnSession;
private:
    typedef yrpc::coroutine::poller::Epoller    Epoller;
    typedef yrpc::util::lock::CountDownLatch    CountDownLatch;
    typedef yrpc::detail::net::Acceptor         Acceptor;
    typedef yrpc::detail::net::ConnectionPtr    ConnPtr;
    typedef yrpc::util::lock::Mutex             Mutex;
    typedef yrpc::detail::net::YAddress         Address;
    typedef yrpc::detail::net::Connector        Connector;
    // typedef std::unordered_map<std::string,SessionID>       AddressMap;
    // typedef std::pair<SessionID,SessionPtr>     Entry;
    typedef std::unordered_map<SessionID,SessionPtr>   SessionMap;
    template<class T>
    using lock_guard = yrpc::util::lock::lock_guard<T>;
public:
    static __YRPC_SessionManager* GetInstance(int n=0);   

    /**
     * 异步的建立Session，Session建立完成，通过onsession返回 
     * 
     * 这里有点东西的，主要是我搞了个协议复用，本质上就是想让两个服务器之间互相提供服务
     * 不去建立两条TCP信道，而是复用一条信道，所以加了一系列复杂机制。
     * 
     * RpcClient 这里分几种情况：
     * 1、如果连接在AddressMap中不存在，说明没有本条连接，则需要重新建立
     * 2、如果连接在AddressMap中，直接返回 RpcSession ，RpcSession 本身就已经可以做到协议分解了，RpcClient就可以持有
     *    RpcSession 进行可靠的通信。
     * 3、如果 多个 RpcClient 同时调用AsyncConnect，就会导致注册多个两机之间的TCP连接。（待解决，标志位或者更好的办法）
     *    这里思考主要是要不要允许可以创建多个rpcclient，指向同一个连接。如果可以那么RpcSession就需要加锁。
     * 
     * 解决方案：
     *    通过两个Map  address to id 和 id to session 区分已建立连接和尚未建立完毕。其实是个简单的状态，如果存在于
     * addressid但是不存在与SessionMap中，说明正在建立连接，连接尚未完成。但是删除连接是完整的删除。建立连接分两阶段:
     *      1、注册连接建立。 address : id
     *      2、建立连接完毕。 id : session
     *    删除还是一个完整的原子操作。好处就是可以通过id去索引 RpcClinet 注册的回调.
     */
    void AsyncConnect(Address peer,OnSession onsession);

    
    
private:
    __YRPC_SessionManager(int Nthread);  

    SessionID GetSessionID();

    // 运行在 main loop 中的，只做新连接的分发
    void RunInMainLoop();
    // 运行在 sub loop 中的，只做io、协议解析
    void RunInSubLoop(Epoller*);
    // 被连接后,
    // void OnAccept(Channel::ChannelPtr,void*);
    // 连接建立
    void OnConnect(Channel::ChannelPtr);
    // 注意这是线程不安全的,获取一个新的session uid
    SessionID GetNewID()
    { return (++m_id_key); }

    SessionID AddressToID(const Address&);
private:

    /**
     * 建立新连接多阶段操作
     */
    // 此操作线程安全: 添加一个新的Session 到 SessionMap 中
    SessionPtr AddNewSession(Channel::ConnPtr newconn);
    // 此操作线程安全: 删除并释放 SessionMap 中一个Session 的资源。如果不存在，则返回false，否则返回true
    bool DelSession(const Address&);
private:
    Epoller*            m_main_loop;        // 只负责 listen 的 epoll
    Acceptor            m_main_acceptor;    // listen 
    Connector           m_connector;        
    Epoller**           m_sub_loop;         // sub eventloop
    const size_t        m_sub_loop_size;    // sub eventloop 数量
    CountDownLatch      m_loop_latch;       // 
    int                 m_balance;          // 新连接轮转负载
    
    std::thread*        m_main_thread;      
    std::thread**       m_sub_threads;   

    SessionMap          m_sessions;         // 会话
    Mutex               m_mutex_sessions;   


    std::hash<std::string>  m_addrhash;     // 地址hash

    uint64_t            m_id_key{10000};  // session id 起名用的

    // main loop 控制
    std::atomic_bool    m_run;

    const int port;
    
};

}
