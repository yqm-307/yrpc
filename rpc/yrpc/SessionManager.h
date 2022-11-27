// #include "RpcClientSession.h"
#include "RpcSession.h"
#include "../network/all.h"
#include "ServiceModule.h"
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
    typedef yrpc::detail::net::YAddress         YAddress;
    typedef yrpc::detail::net::Connector        Connector;
    typedef yrpc::detail::net::errorcode        errorcode;
    typedef yrpc::detail::net::ConnectionPtr    ConnectionPtr;
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
    bool AsyncConnect(YAddress peer,OnSession onsession);

    
    /**
     * 创建服务器地址，设置服务提供方处理函数
     * 
     * 负载均衡: 轮转法
     * 
     * Todo : 被连接也要保存到SessionMap 
     */
    void AsyncAccept(const YAddress& peer);
private:
    __YRPC_SessionManager(int Nthread);  

    SessionID GetSessionID();

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

    SessionID AddressToID(const YAddress&);

    Epoller* LoadBalancer();
private:

    /**
     * 建立新连接多阶段操作
     */
    // thread unsafe: 添加一个新的Session 到 SessionMap 中
    SessionPtr AddNewSession(Channel::ConnPtr newconn);
    // 此操作线程安全: 删除并释放 SessionMap 中一个Session 的资源。如果不存在，则返回false，否则返回true
    bool DelSession(const YAddress&);

    void Dispatch(const std::string &string, SessionPtr sess);


    /**
     *  等待连接成功队列
     */
    struct ConnectWaitQueue_Impl
    {
        typedef std::unique_ptr<ConnectWaitQueue_Impl> Ptr;

        /**
         * @brief 寻找是否有正在连接 key 的任务正在进行
         * 
         * @param key   正在进行中的连接任务的地址 
         * @return auto 
         */
        auto Find(const YAddress &key)
        {
            auto id = AddressToID(key);
            return m_map.find(id);
        }

        /**
         * @brief 查找是否已经注册连接任务，如果没有注册则注册任务
         * 
         * @param addr  服务端地址
         * @param func  回调函数
         * @return int  如果尚未存在则插入并返回 ID，如果已经存在返回-1
         */
        template<typename OnSession>
        uint64_t FindAndInsert(const YAddress& addr,OnSession&& func)
        {
            SessionID ret = 0;
            do
            {
                ret = AddressToID(addr);
                auto it = m_map.find(ret);
                if (it != m_map.end())
                {
                    ret = -1; // 已经存在
                    break;
                }
                else
                    m_map.insert(std::make_pair(ret, func));
            } while (0);

            return ret;
        }

        /**
         * @brief 将 address 转化为唯一的SessionID并返回
         */
        SessionID AddressToID(const YAddress&key)
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


        OnSession FindAndRemove(const YAddress& key)
        {
            return FindAndRemove(AddressToID(key));
        }


        OnSession FindAndRemove(SessionID id)
        {
            auto it = m_map.find(id);
            if(it == m_map.end())
                return nullptr;
            else
                return it->second;
        }





        std::map<SessionID,OnSession>   m_map;
        Mutex                           m_mutex;
    };




private:
    Epoller*            m_main_loop;        // 只负责 listen 的 epoll
    Acceptor*           m_main_acceptor;    // listen 
    Connector           m_connector;        
    Epoller**           m_sub_loop;         // sub eventloop
    const size_t        m_sub_loop_size;    // sub eventloop 数量
    CountDownLatch      m_loop_latch;       // 
    std::atomic_int     m_balance;          // 新连接轮转负载，跨线程（需要atomic，还需要考虑 memory order）
    
    std::thread*        m_main_thread;      
    std::thread**       m_sub_threads;   

    SessionMap          m_sessions;         // 会话
    Mutex               m_mutex_sessions;   


    std::hash<std::string>  m_addrhash;     // 地址hash

    uint64_t            m_id_key{10000};  // session id 起名用的

    // main loop 控制
    std::atomic_bool    m_run;

    ConnectWaitQueue_Impl::Ptr  m_conn_queue;
    
    const int port;  
};
}