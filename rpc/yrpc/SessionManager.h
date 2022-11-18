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
    typedef std::function<void(SessionPtr)>     OnSession;
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
     * 解决方案：
     *    通过两个Map  address to id 和 id to session 区分已建立连接和尚未建立完毕。其实是个简单的状态，如果存在于
     * addressid但是不存在与SessionMap中，说明正在建立连接，连接尚未完成。但是删除连接是完整的删除。建立连接分两阶段:
     *      1、如果已经正在Connect中，则返回false
     *    删除还是一个完整的原子操作。好处就是可以通过id去索引 RpcClinet 注册的回调.
     */
    bool AsyncConnect(Address peer,OnSession onsession);

    
    /**
     * 创建服务器地址，设置服务提供方处理函数
     * 
     * Todo : 被连接也要保存到SessionMap 
     */
    void AsyncAccept(Address peer,RpcSession::DispatchCallback dspcb);
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


    struct Service_Impl
    {
        
        void Init(RpcSession::DispatchCallback callback)
        { m_dispatch = callback;  m_started.store(true); }

        bool IsServiceSupplier()
        { return m_started.load(); }

        auto GetDispatchHandler()
        { return m_dispatch; }

        std::atomic_bool                m_started{false};
        RpcSession::DispatchCallback    m_dispatch;
    };


    /**
     *  等待连接成功队列
     */
    struct ConnectWaitQueue_Impl
    {
        /**
         * @brief 寻找是否有正在连接 key 的任务正在进行
         * 
         * @param key   正在进行中的连接任务的地址 
         * @return auto 
         */
        auto Find(const Address &key)
        {
            auto id = AddressToID(key);
            return m_map.find(id);
        }

        SessionID AddressToID(const Address&key)
        {
            auto str = key.GetIPPort();
            std::string id(19, '0');
            int j = 0;
            for (int i = 0; i < str.size(); ++i)
            {
                if (str[i] >= '0' && str[i] <= '9')
                {
                    id[j++] = str[i];
                }
            }
            return std::stoull(id);
        }







        std::map<SessionID,OnSession> m_map;
    };




private:
    Epoller*            m_main_loop;        // 只负责 listen 的 epoll
    Acceptor*           m_main_acceptor;    // listen 
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

    Service_Impl        m_server;
    

    const int port;  
};
}