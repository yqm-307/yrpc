#include "RpcClientSession.h"
#include "RpcSession.h"
#include <vector>
#include <map>

namespace yrpc::rpc::detail
{

/**
 * @brief session 管理器，客户端最重要的管理模块
 * 1、负载均衡加在这里
 * 2、服务探测也加在这里
 * 3、心跳检测也加在这里
 * 4、承上启下，上对应RpcClient的使用者rpc call，下对应 network io
 */
class SessionManager : yrpc::util::noncopyable::noncopyable
{
public:
    typedef uint64_t SessionID;
    typedef std::function<void(RpcSession*)>    OnSession;
private:
    typedef yrpc::coroutine::poller::Epoller    Epoller;
    typedef yrpc::util::lock::CountDownLatch    CountDownLatch;
    typedef yrpc::detail::net::Acceptor         Acceptor;
    typedef yrpc::detail::net::ConnectionPtr    ConnPtr;
    typedef yrpc::util::lock::Mutex             Mutex;
    typedef yrpc::detail::net::YAddress         Address;
    typedef yrpc::detail::net::Connector        Connector;
    typedef std::unordered_map<std::string,SessionID>       AddressMap;
    typedef std::pair<SessionID,RpcSession*>    Entry;
    typedef std::unordered_map<SessionID,RpcSession*>   SessionMap;
    template<class T>
    using lock_guard = yrpc::util::lock::lock_guard<T>;
public:
    static SessionManager* GetInstance(int n=0);   

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
     */
    void AsyncConnect(Address peer,OnSession onsession);

    
    
private:
    SessionManager(int Nthread);  

    SessionID GetSessionID();

    // 运行在 main loop 中的，只做新连接的分发
    void RunInMainLoop();
    // 运行在 sub loop 中的，只做io、协议解析
    void RunInSubLoop(Epoller*);
    // 连接建立
    void OnAccept(ConnPtr,void*);

    void OnConnect(ConnPtr);

private:
    // 添加一个新的Session 到 SessionManager 的数据结构中
    void AddNewSession(Address,RpcSession*);
    // 删除并释放 Session 中一个Session 的资源。如果不存在，则返回false，否则返回true
    bool DelSession(SessionID);
private:
    Epoller*            m_main_loop;        // 只负责 listen 的 epoll
    Acceptor            m_main_acceptor;    // listen 
    Connector           m_connector;        
    Epoller**           m_sub_loop;         // 协程调度器
    CountDownLatch      m_loop_latch;       // 
    int                 m_balance;          // 新连接轮转负载
    
    std::thread*        m_main_thread;
    std::thread**       m_sub_threads;   


    SessionMap          m_sessions;         // 会话
    AddressMap          m_addrtoid;         
    Mutex               m_mutex;


    std::hash<std::string>  m_addrhash;     // 地址hash

    std::atomic<uint64_t> m_id_key{10000};

    // main loop 控制
    std::atomic_bool    m_run;

    const int port;
    
};

}
