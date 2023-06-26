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
    m_undone_conn_queue(std::make_unique<ConnQueue>())

{
    m_node_id = bbt::uuid::UuidMgr::CreateUUid();
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
    delete m_main_acceptor;
    m_main_acceptor = nullptr;
    ERROR("[YRPC][~__YRPC_SessionManager] session manager destory!");
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
    auto sessionptr = RpcSession::Create(channelptr,nextloop);
    // 创建并初始化新Session
    sessionptr->SetCloseFunc([this](const yrpc::detail::shared::errorcode& e,const yrpc::detail::net::YAddress& addr){
        // 连接断开，从SessionMap中删除此Session
        if (this->DelSession(addr))
        {
            INFO("[YRPC][__YRPC_SessionManager] session closed! ,%s", addr.GetIPPort().c_str());
        }
        else{
            WARN("[YRPC][__YRPC_SessionManager] session closed! ,%s ,but not exist in SessionManager!", addr.GetIPPort().c_str());
        }
    });
    // 超时
    sessionptr->SetTimeOutFunc([sessionptr](Socket* socket){
        // 实际上还没有使用
        if (!sessionptr->IsClosed())
            sessionptr->Close();    // 触发 CloseCallback
    });

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

SessionPtr __YRPC_SessionManager::AddUnDoneSession(ConnPtr new_conn)
{
    auto session_ptr = InitRpcSession(new_conn);
    struct yrpc::rpc::detail::HandShakeData hand_shake_data;
    hand_shake_data.m_succ = [this](const yrpc::detail::shared::errorcode& e, SessionPtr sess){
        OnHandShakeSucc(e, sess);
    };
    // 加入到半连接队列
    int status = m_undone_conn_queue->FindAndPush(new_conn->GetPeerAddress(), hand_shake_data);
    if( status > 0 )
    {
        return session_ptr;
    }
    return nullptr;
}


bool __YRPC_SessionManager::DelSession(UuidPtr peer_uuid)
{    
    lock_guard<Mutex> lock(m_mutex_session_map);
    auto its = m_session_map.find(peer_uuid);
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
        const int * ptr = nullptr;
        int thread_num = (ptr = BBT_CONFIG()->GetDynamicCfg()->GetEntry<int>(yrpc::config::SysCfg[yrpc::config::THREAD_NUM])) == nullptr ?
                sysconf(_SC_NPROCESSORS_ONLN)*2 : *ptr;
        manager = new __YRPC_SessionManager(thread_num);  // 默认根据处理器数量*2 分配线程
    }
    return manager;
}

void __YRPC_SessionManager::OnAccept(const errorcode &e, ConnectionPtr conn)
{
    /* 1、创建Session，保存在undone queue中 */
    auto new_sess_ptr = this->AddUnDoneSession(conn);

    /* 2、设置超时定时器 */
    /* todo: 将握手超时配置化 */
    new_sess_ptr->StartHandShakeTimer([this](const yrpc::detail::shared::errorcode& e, SessionPtr sess){
        OnHandShakeTimeOut(e, sess);
    }, 3000);

    /* 连接完全完成后，暂时保留以供参考 */
    // SessionID tmpid = AddressToID(conn->GetPeerAddress());
    // if ( e.err() == yrpc::detail::shared::ERR_NETWORK_ACCEPT_OK )
    // {
    //     lock_guard<Mutex> lock(m_mutex_session_map);
    //     // 查询是否已经存在连接
    //     auto it = m_session_map.find(tmpid);
    //     if ( it == m_session_map.end() )
    //     {
    //         auto newsess = this->AddNewSession(conn);
    //         // 检查一下是否有主动连接对端的任务
    //         auto [onsessfunc,succ] = m_undone_conn_queue->PopUpById(tmpid);
    //         if (succ)
    //         {
    //             if ( onsessfunc != nullptr )
    //                 onsessfunc(newsess);
    //             INFO("[YRPC][__YRPC_SessionManager::OnAccept] SessionID: %ld , session connect repeat",tmpid);
    //         }
    //     }
    //     else
    //     {
    //         /**
    //          * 如果连接已经存在，那么就复用，只允许两端之间存在一条tcp连接
    //          */
    //         conn->Close();
    //         WARN("[YRPC][__YRPC_SessionManager::OnAccept] SessionID: %ld , session is exist",tmpid);
    //     }

    // }
    // else{
    //     ERROR("[YRPC][__YRPC_SessionManager::OnAccept] %s",e.what().c_str());
    // }
}

void __YRPC_SessionManager::OnConnect(const errorcode &e, const Address& addr, ConnectionPtr conn)
{
    // 无论成功失败
    {
        lock_guard<Mutex> lock(m_mutex_session_map);
        if( ! m_undone_conn_queue->DelTcpConn(addr) )
        {
            WARN("[YRPC][__YRPC_SessionManager::OnConnect] tcp connect not exist!");
        }
    }
    if (e.err() == yrpc::detail::shared::ERR_NETWORK_CONN_OK)
    {
        SessionPtr new_sess_ptr{nullptr};
        {
            lock_guard<Mutex> lock(m_mutex_session_map);
            if( ! m_undone_conn_queue->DelTcpConn(addr) )
            {
                WARN("[YRPC][__YRPC_SessionManager::OnConnect] tcp connect not exist!");
            }
            // 1、创建新session，保存到半连接队列
            auto new_sess_ptr = AddUnDoneSession(conn);
        }
        if( new_sess_ptr == nullptr )
        {
            conn->Close();
            ERROR("[YRPC][__YRPC_SessionManager::OnConnect] session create error!");
            return;
        }
        // 2、发送握手请求
        StartHandShake(e, new_sess_ptr);
    }
    else 
    {
        ERROR("[YRPC][__YRPC_SessionManager::OnConnect] connect error!");
    }
}

void __YRPC_SessionManager::AsyncConnect(Address peer_addr,OnSession onsession)
{
    using namespace yrpc::detail::net;
    bool is_need_connect{false};

    lock_guard<Mutex> lock(m_mutex_session_map);
    auto it_uuid = m_knownode_map.find(peer_addr);
    if( it_uuid != m_knownode_map.end() )
    {// 此节点是已知节点
        auto it_done_sess = m_session_map.find(it_uuid->second);
        if( it_done_sess == m_session_map.end() )
        {// 已完成连接中不存在Session
            auto it_conned_sess = m_undone_conn_queue->Find(peer_addr);
            if( it_conned_sess == nullptr )
            {// 连接不存在，新建连接
                is_need_connect = true;
            }
        }
        else
        {// 已存在session，回调通知
            onsession(it_done_sess->second);
        }
    }
    else
    {// 连接一个未知节点
        auto it_undone_sess = m_undone_conn_queue->Find(peer_addr);
        if( it_undone_sess == nullptr )
        {// 半连接队列中不存在，需发起连接
            is_need_connect = true;
        }
    }
    if( is_need_connect )
    {
        Socket* socket = Connector::CreateSocket();
        m_undone_conn_queue->AddTcpConn(peer_addr);
        m_connector->AsyncConnect(socket, peer_addr, functor([this](const errorcode& e, const Address& peer_addr, ConnectionPtr conn){
            OnConnect(e, peer_addr, conn);
        }));
    }
}

int __YRPC_SessionManager::Connect(const Address& peer_addr, SessionPtr sess)
{


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
    assert(y_scheduler != nullptr);
    m_sub_loop[idx] = y_scheduler;
    m_sub_loop[idx]->RunForever();
    m_loop_latch.down();
    m_loop_latch.wait();
    m_sub_loop[idx]->Loop();
}
void __YRPC_SessionManager::MainLoop()
{
    assert(y_scheduler != nullptr);
    m_main_loop = y_scheduler;
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


MessagePtr __YRPC_SessionManager::Handler_HandShake(MessagePtr, const SessionPtr sess)
{
    // todo 
    // 1、保存数据到 node list
    // 2、返回握手成功结果 + 数据
    // 3、连接成功，操作 m_session_map
    // 4、清空超时定时器
    WARN("[YRPC][__YRPC_SessionManager::Handler_HandShake] 函数未实现!");
}

void __YRPC_SessionManager::StartHandShake(const yrpc::detail::shared::errorcode& e, SessionPtr sess)
{
    // todo
    // 1、发送开始握手请求
    // 2、携带数据发送
    // 3、设置超时定时器
    WARN("[YRPC][__YRPC_SessionManager::StartHandShake] 函数未实现!");
}

void __YRPC_SessionManager::OnHandShakeSucc(const yrpc::detail::shared::errorcode& e, SessionPtr sess)
{
    WARN("[YRPC][__YRPC_SessionManager::OnHandShakeSucc] 函数未实现!");
}

void __YRPC_SessionManager::OnHandShakeTimeOut(const yrpc::detail::shared::errorcode& e, SessionPtr sess)
{
    WARN("[YRPC][__YRPC_SessionManager::OnHandShakeTimeOut] 函数未实现!");
}

SessionPtr __YRPC_SessionManager::InitRpcSession(ConnPtr new_conn)
{
    auto next_loop = m_sub_loop[BalanceNext];
    auto channel_ptr = Channel::Create(new_conn, next_loop);
    auto session_ptr = RpcSession::Create(channel_ptr, next_loop);

    // 初始化新Session的回调
    session_ptr->SetCloseFunc([this](const yrpc::detail::shared::errorcode& e, SessionPtr addr){
        OnSessionClose(e, addr);
    });
    session_ptr->SetTimeOutFunc([this](const yrpc::detail::shared::errorcode& e, SessionPtr addr){
        OnSessionTimeOut(e, addr);
    });
    session_ptr->UpdataAllCallbackAndRunInEvloop();
    return session_ptr;
}

#undef BalanceNext