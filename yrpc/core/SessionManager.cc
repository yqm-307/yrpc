#include "SessionManager.h"
#include "yrpc/config/config.hpp"
#include <unistd.h>
using namespace yrpc::rpc::detail;

#define BalanceNext ( m_balance = (m_balance + 1) % (m_sub_loop_size))


__YRPC_SessionManager::__YRPC_SessionManager(int Nthread)
    :m_main_loop(nullptr),
    m_main_acceptor(nullptr),
    m_connector(nullptr),
    m_sub_loop_size(Nthread - 1),
    m_sub_loop(m_sub_loop_size),
    m_loop_latch(Nthread),
    m_undone_conn_queue(std::make_unique<ConnQueue>()),
    port(7912)
{
    // 初始化 main eventloop，但是不运行
    m_main_thread = new std::thread([this](){
        this->MainLoop();
    });

    // 初始化 sub eventloop，并运行（由于队列为空，都挂起）
    assert(Nthread>=2);
    for (int i=0;i<Nthread - 1;++i)
    {
        m_sub_threads.push_back(new std::thread([this,i](){
            this->SubLoop(i);
        }));
    }
    m_loop_latch.wait();    
    INFO("[YRPC][__YRPC_SessionManager] info: SessionManager Init Success!");
}

__YRPC_SessionManager::~__YRPC_SessionManager()
{
    // for(auto && ptr : m_sub_loop)
    // {
    //     delete ptr;
    // }
    delete m_main_acceptor;
    m_main_acceptor = nullptr;
    // delete m_main_loop;
}



SessionPtr __YRPC_SessionManager::AddNewSession(Channel::ConnPtr connptr)
{
    /**
     * 建立新连接
     * 
     * 建立新连接，并保存在Manager中，注意注册超时、关闭时回调
     */
    auto nextloop = m_sub_loop[BalanceNext];
    auto channelptr = Channel::Create(connptr,nextloop);
    auto sessionptr = RpcSession::Create(channelptr,m_sub_loop[BalanceNext]);
    // 创建并初始化新Session
    sessionptr->SetCloseFunc([this](const yrpc::detail::shared::errorcode& e,const yrpc::detail::net::YAddress& addr){
        // 连接断开，从SessionMap中删除此Session
        if (this->DelSession(addr))
        {
            INFO("[YRPC][__YRPC_SessionManager] session closed!");
        }
        else{
            WARN("[YRPC][__YRPC_SessionManager] session closed! but not exist in SessionManager!");
        }
    });
    // 超时
    sessionptr->SetTimeOutFunc([sessionptr](Socket* socket){
        // 实际上还没有使用
        if (!sessionptr->IsClosed())
            sessionptr->Close();    // 触发 CloseCallback
    });

    // 设置服务处理函数
    sessionptr->SetToServerCallback([this](Buffer&&pck, SessionPtr ptr)
                                        { this->Dispatch(std::forward<Buffer>(pck), ptr); });
    /**
     *  这里要支持连接复用，要检查是否已经建立了和目标进程的连接（目标进程标识{ip:port}）
     */
    auto it = m_session_map.insert(std::make_pair(AddressToID(channelptr->GetConnInfo()->GetPeerAddress()),sessionptr));        // session 映射
    
    if( !it.second )
    {
        sessionptr->Close();
        return it.first->second;
    }

    sessionptr->UpdataAllCallbackAndRunInEvloop();
    
    return sessionptr;
}

bool __YRPC_SessionManager::DelSession(const Address& id)
{    
    lock_guard<Mutex> lock(m_mutex_session_map);
    auto its = m_session_map.find(AddressToID(id));
    if ( its != m_session_map.end() )
    {
        m_session_map.erase(its);
        return true;
    }
    else
        return false;   // 不存在此节点
}


void __YRPC_SessionManager::RunInMainLoop()
{
    assert(m_main_acceptor != nullptr);
    m_main_acceptor->StartListen();
}

void __YRPC_SessionManager::RunInSubLoop(Epoller* lp)
{
    lp->Loop();
}

// void __YRPC_SessionManager::OnAccept(Channel::ChannelPtr newchannel,void* ep)
// {
//     /**
//      * 被连接后,注册到SessionMap中
//      */
//     AddNewSession(newchannel);
// }



__YRPC_SessionManager *__YRPC_SessionManager::GetInstance()
{
    static __YRPC_SessionManager *manager = nullptr;
    if (manager == nullptr)
    {
        int *ptr = nullptr;
        int thread_num = (ptr = BBT_CONFIG()->GetDynamicCfg()->GetEntry<int>(yrpc::config::SysCfg[yrpc::config::THREAD_NUM])) == nullptr ?
                sysconf(_SC_NPROCESSORS_ONLN)*2 : *ptr;
        manager = new __YRPC_SessionManager(thread_num);  // 默认根据处理器数量*2 分配线程
    }
    return manager;
}

void __YRPC_SessionManager::OnAccept(const errorcode &e, ConnectionPtr conn)
{
    /**
     *  讨论一个小概率事件(tcp中也有这种小概率事件):
     *  如果A 、 B两个进程之间建立连接，同时注册，tcp握手过程中，应用层是没法及时反馈。那么怎么复用连接？
     *  
     *  1、当有连接建立完成，立即插入SessionMap中（加锁的），这样另一个发现SessionMap中已经有RpcSession了
     *    就立即放弃当前准备插入的连接。这样修改 AddNewSession 接口即可。
     */
    SessionID tmpid = AddressToID(conn->GetPeerAddress());
    if ( e.err() == yrpc::detail::shared::ERR_NETWORK_ACCEPT_OK )
    {
        lock_guard<Mutex> lock(m_mutex_session_map);
        // 查询是否已经建立，否则释放此连接，复用已经存在的连接
        auto it = m_session_map.find(tmpid);
        if ( it == m_session_map.end() )
        {
            auto newsess = this->AddNewSession(conn);
            // 检查一下是否有主动连接对端的任务
            auto [onsessfunc,succ] = m_undone_conn_queue->PopUpById(tmpid);
            if (succ)
            {
                if ( onsessfunc != nullptr )
                    onsessfunc(newsess);
                INFO("[YRPC][__YRPC_SessionManager::OnAccept] SessionID: %ld , session connect repeat",tmpid);
            }
        }
        else
        {
            /**
             * 如果连接已经存在，那么就复用，只允许两端之间存在一条tcp连接
             */
            conn->Close();
            WARN("[YRPC][__YRPC_SessionManager::OnAccept] SessionID: %ld , session is exist",tmpid);
        }

    }
    else{
        ERROR("[YRPC][__YRPC_SessionManager::OnAccept] %s",e.what().c_str());
    }
}
void __YRPC_SessionManager::OnConnect(const errorcode &e, ConnectionPtr conn)
{
    // 构造新的Session
    if (e.err() == yrpc::detail::shared::ERR_NETWORK_CONN_OK)
    {
        // 更新SessionMap
        SessionPtr newSession{nullptr}; 
        {
            lock_guard<Mutex> lock(m_mutex_session_map);
            newSession = this->AddNewSession(conn);
            SessionID tmpid = AddressToID(conn->GetPeerAddress());
            // clean 连接等待队列，处理回调任务
            auto [onsessfunc,exist] = this->m_undone_conn_queue->PopUpById(tmpid);
            auto done_session = m_session_map.find(tmpid);
            if ( exist )
            {
                if (done_session != m_session_map.end())
                {
                    newSession->Close();
                    WARN("[YRPC][__YRPC_SessionManager::OnConnect] The connection may already exist!");
                }
                else{
                    if (onsessfunc != nullptr)
                        onsessfunc(newSession);
                }
            }
            else
            {
                if (done_session != m_session_map.end())
                {
                    newSession->Close();
                    WARN("[YRPC][__YRPC_SessionManager::OnConnect] The connection may already exist! but exist two queue :D?");
                }
                else
                    ERROR("[YRPC][__YRPC_SessionManager::OnConnect] connection may be not exist!\
                    because not in m_session_map or m_undone_conn_queue!");
            }
        }
    }
    else
    {
        /* todo 网络连接失败错误处理 */
        ERROR("[YRPC][__YRPC_SessionManager::OnConnect] connect error!");
    }
}

int __YRPC_SessionManager::AsyncConnect(Address peer,OnSession onsession)
{
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
    using namespace yrpc::detail::net;

    int ret = -1;    
    SessionID tmpid = AddressToID(peer);
    lock_guard<Mutex> lock(m_mutex_session_map);
    auto it = m_session_map.find(tmpid);

    /**
     * 1、如果sessions中已经有了该连接，则直接返回即可
     * 2、如果sessions中没有，则检查是否已经注册连接事件
     *      (1) 如果 connect async wait queue 中没有对应的用户会话，注册一个新的connect事件
     *      (2) 如果 connect async wait queue 中有对应的用户会话且注册失败，返回false
     */
    if( it == m_session_map.end() )
    {
        Socket* socket = Connector::CreateSocket();
        auto status = m_undone_conn_queue->FindAndPush(tmpid,onsession);
        if( status <= 1 ) // 存在或者插入失败
        {
            ret = -1;
        }
        else
        {
            m_connector->AsyncConnect(socket, peer, functor([this](const errorcode &e, ConnectionPtr conn)
            {
                this->OnConnect(std::forward<const errorcode>(e),std::forward<ConnectionPtr>(conn));
            }));
            ret = 0;
        }
    }
    else
    {
        if (onsession != nullptr)
            onsession(it->second);
        else
            WARN("[YRPC][__YRPC_SessionManager::AsyncConnect] async connect success callback is nullptr, maybe warning");
        ret = 1;
    }

    return ret;
}


void __YRPC_SessionManager::AsyncAccept(const Address& peer)
{
    /**
     * 创建服务器地址，设置服务提供方处理函数
     * 
     * 负载均衡: 轮转法
     * 
     * Todo : 被连接也要保存到SessionMap 
     */
    if(m_main_acceptor != nullptr)
    {
        delete m_main_acceptor;
        m_main_acceptor = nullptr;
    }
    m_main_acceptor = new Acceptor(peer.GetPort(),3000,5000);
    
    m_main_acceptor->setLoadBalancer(functor([this]()->auto {return this->LoadBalancer(); }));
    m_main_acceptor->setOnAccept(functor([this](const yrpc::detail::shared::errorcode&e,yrpc::detail::net::ConnectionPtr conn)->void{
        this->OnAccept(e,conn);
    }),nullptr);
    m_main_loop->AddTask([this](void*){
        this->RunInMainLoop();
    });
}

Epoller* __YRPC_SessionManager::LoadBalancer()
{
    return m_sub_loop[BalanceNext];
}

void __YRPC_SessionManager::Dispatch(Buffer&&string, SessionPtr sess)
{
    yrpc::rpc::detail::Service_Base::GetInstance()->Dispatch(std::forward<Buffer>(string), sess);
}

SessionID __YRPC_SessionManager::AddressToID(const Address&key)
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


void __YRPC_SessionManager::SubLoop(int idx)
{
    assert(_co_scheduler != nullptr);
    m_sub_loop[idx] = _co_scheduler;
    m_sub_loop[idx]->RunForever();
    m_loop_latch.down();
    m_loop_latch.wait();
    m_sub_loop[idx]->Loop();
}
void __YRPC_SessionManager::MainLoop()
{
    assert(_co_scheduler != nullptr);
    m_main_loop = _co_scheduler;
    m_connector = new Connector(m_main_loop);
    m_main_loop->RunForever();
    m_loop_latch.down();
    m_loop_latch.wait();
    m_main_loop->Loop();
}

SessionPtr __YRPC_SessionManager::TryGetSession(const Address& peer)
{
    SessionID sid = AddressToID(peer);
    lock_guard<Mutex> lock(m_mutex_session_map);
    auto it = m_session_map.find(sid);
    if (it == m_session_map.end())
        return nullptr;
    else
        return it->second;
}

bool __YRPC_SessionManager::IsConnecting(const Address& peer)
{
    SessionID sid = AddressToID(peer);
    lock_guard<Mutex> lock(m_mutex_session_map);
    auto it = m_undone_conn_queue->Find(sid);
    if (it == nullptr)
        return false;
    else
        return true;
}


#undef BalanceNext