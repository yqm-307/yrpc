#include "SessionManager.h"
#include "yrpc/config/config.hpp"
#include "yrpc/protocol/protoc/c2s.pb.h"
#include "yrpc/protocol/protoc/s2c.pb.h"
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
    m_knownode_map()
{
    m_local_node_id = bbt::uuid::UuidMgr::CreateUUid();
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
    INFO("[YRPC][__YRPC_SessionManager][%d] info: SessionManager Init Success!", y_scheduler_id);
}

__YRPC_SessionManager::~__YRPC_SessionManager()
{
    m_main_acceptor = nullptr;
    ERROR("[YRPC][~__YRPC_SessionManager][%d] session manager destory!", y_scheduler_id);
}

void __YRPC_SessionManager::RegisterService()
{
    yrpc::rpc::Rpc::register_service<C2S_HANDSHAKE_REQ, S2C_HANDSHAKE_RSP>("YRPC_HandShake", [=](const MessagePtr msg, SessionPtr sess_ptr){
        return Handler_HandShake(msg, sess_ptr);
    });
}


SessionPtr __YRPC_SessionManager::Append_SessionMap(bbt::uuid::UuidBase::Ptr uuid, SessionPtr sess)
{
    if( sess == nullptr )
        return nullptr;
    auto [it, succ] = m_session_map.insert(std::make_pair(uuid, sess));
    if( !succ )
    {
        return nullptr;
    }
    return sess;
}

SessionPtr __YRPC_SessionManager::Delete_SessionMap(bbt::uuid::UuidBase::Ptr uuid)
{
    auto it = m_session_map.find(uuid);
    if( it != m_session_map.end() )
    {
        it = m_session_map.erase(it);
        return ( (it == m_session_map.end()) ? nullptr : it->second );
    }
    else
        return nullptr;
}

SessionPtr __YRPC_SessionManager::Append_UnDoneMap(RpcSession::SPtr new_sess)
{
    // auto session_ptr = InitRpcSession(new_chan);
    struct yrpc::rpc::detail::HandShakeData hand_shake_data;
    hand_shake_data.m_sess = new_sess;
    hand_shake_data.m_succ = [this](const yrpc::detail::shared::errorcode& e, SessionPtr sess){
        OnHandShakeFinal(e, sess);
    };
    // 加入到半连接队列
    int status = m_undone_conn_queue->FindAndPush(new_sess->GetPeerAddress(), hand_shake_data);
    if( status > 0 )
    {
        return new_sess;
    }
    return nullptr;
}

bool __YRPC_SessionManager::Delete_UnDoneMap(const Address& peer_addr)
{
    return  m_undone_conn_queue->DelTcpConn(peer_addr);
}

SessionPtr __YRPC_SessionManager::GetSessionFromSessionMap(bbt::uuid::UuidBase::Ptr uuid)
{
    auto it = m_session_map.find(uuid);
    if( it == m_session_map.end() )
    {
        return nullptr;
    }
    return it->second;
}

void __YRPC_SessionManager::RunInMainLoop()
{
    assert(m_main_acceptor != nullptr);
    m_channel_mgr.StartListen();
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

void __YRPC_SessionManager::OnAccept(const errorcode &e, Channel::SPtr chan)
{
    if( e.err() == yrpc::detail::shared::ERR_NETWORK_ACCEPT_OK )
    {
        /* 1、创建Session，保存在undone queue中 */
        auto new_sess_ptr = InitRpcSession(chan);
        auto sess_ptr = this->Append_UnDoneMap(new_sess_ptr);
        if( sess_ptr == nullptr )
        {
            ERROR("[YRPC][__YRPC_SessionManager::OnAccept][%d] append undone map failed! addr: {%s}", y_scheduler_id, chan->GetPeerAddress().GetIPPort());
            return;
        }
        /* 2、设置超时定时器 */
        /* todo: 将握手超时配置化 */
        new_sess_ptr->StartHandShakeTimer([this](const yrpc::detail::shared::errorcode& e, SessionPtr sess){
            OnHandShakeTimeOut(e, sess);
        }, 3000);
    }
    else
    {
        ERROR("[YRPC][__YRPC_SessionManager::OnAccept][%d] accept failed! msg: %s", y_scheduler_id, e.what().c_str());
    }
}

void __YRPC_SessionManager::OnConnect(const errorcode &e, Channel::SPtr chan)
{
    if (e.err() == yrpc::detail::shared::ERR_NETWORK_CONN_OK)
    {
        auto addr = chan->GetConnInfo()->GetPeerAddress();
        SessionPtr new_sess_ptr{nullptr};
        {
            lock_guard<Mutex> lock(m_mutex_session_map);
            if( ! m_undone_conn_queue->DelTcpConn(addr) )
            {
                WARN("[YRPC][__YRPC_SessionManager::OnConnect][%d] tcp connect not exist!", y_scheduler_id);
            }
            // 1、创建新session，保存到半连接队列
            auto sess = InitRpcSession(chan);
            new_sess_ptr = Append_UnDoneMap(sess);
        }
        if( new_sess_ptr == nullptr )
        {
            chan->Close();
            ERROR("[YRPC][__YRPC_SessionManager::OnConnect][%d] session create error!", y_scheduler_id);
            return;
        }
        new_sess_ptr->m_current_loop->AddTask([=](void*){
            StartHandShake(e, new_sess_ptr);
        });
    }
    else 
    {
        ERROR("[YRPC][__YRPC_SessionManager::OnConnect][%d] connect error! msg: %s", y_scheduler_id, e.what().c_str());
    }
}

void __YRPC_SessionManager::AsyncConnect(Address peer_addr,OnSession onsession)
{
    using namespace yrpc::detail::net;
    bool is_need_connect{false};

    lock_guard<Mutex> lock(m_mutex_session_map);
    auto it_uuid = m_knownode_map.find(peer_addr);
    // 是否已知节点
    if( it_uuid != m_knownode_map.end() )
    {
        auto it_done_sess = m_session_map.find(it_uuid->second);
        // 是否已有连接
        if( it_done_sess == m_session_map.end() )
            // 是否在半连接队列中
            if( ! m_undone_conn_queue->HasWaitting(peer_addr) )
                is_need_connect = true;
        else
            onsession(it_done_sess->second);
    }
    else
        // 是否在半连接队列中
        if( !m_undone_conn_queue->HasWaitting(peer_addr) )
            is_need_connect;

    if( is_need_connect )
    {
        m_channel_mgr.AsyncConnect(peer_addr);
        m_undone_conn_queue->AddTcpConn(peer_addr);
    }
}

void __YRPC_SessionManager::StartListen(const Address& peer)
{
    /**
     * 创建服务器地址，设置服务提供方处理函数
     * 
     * 负载均衡: 轮转法
     * 
     * Todo : 被连接也要保存到SessionMap 
     */
    if(m_main_acceptor != nullptr)
        return;
    m_main_acceptor = Acceptor::Create(peer.GetPort(),3000,5000);
    
    m_main_acceptor->setLoadBalancer(functor([this]()->auto {return this->LoadBalancer(); }));
    m_channel_mgr.SetAcceptor(m_main_acceptor);
    m_channel_mgr.SetOnAccept([this](const yrpc::detail::shared::errorcode&e, Channel::SPtr chan)->void{
        this->OnAccept(e,chan);
    });
    m_local_addr = peer;
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
    DEBUG("[YRPC][__YRPC_SessionManager::SubLoop][%d] sub loop begin!", y_scheduler_id);
    m_sub_loop[idx]->Loop();
    DEBUG("[YRPC][__YRPC_SessionManager::SubLoop][%d] sub loop begin!", y_scheduler_id);

}
void __YRPC_SessionManager::MainLoop()
{
    assert(y_scheduler != nullptr);
    m_main_loop = y_scheduler;
    OnMainLoopInit();
    m_main_loop->RunForever();
    m_loop_latch.down();
    m_loop_latch.wait();
    DEBUG("[YRPC][__YRPC_SessionManager::MainLoop][%d] main loop begin!", y_scheduler_id);
    m_main_loop->Loop();
    DEBUG("[YRPC][__YRPC_SessionManager::MainLoop][%d] main loop end!", y_scheduler_id);
}

void __YRPC_SessionManager::OnMainLoopInit()
{

    m_connector = Connector::Create(m_main_loop);
    m_connector->setLoadBalancer(functor([this]()->auto{ return LoadBalancer(); }));
    m_channel_mgr.SetOnConnect([this](const errorcode& err, Channel::SPtr new_chan){
        OnConnect(err, new_chan);
    });
    m_channel_mgr.SetConnector(m_connector);
}


SessionPtr __YRPC_SessionManager::TryGetSession(const Address& peer)
{
    lock_guard<Mutex> lock(m_mutex_session_map);
    auto uuid = GetUuid(peer);
    if( nullptr == uuid )
    {
        DEBUG("[YRPC][__YRPC_SessionManager::TryGetSession] uuid not found!");
        return nullptr;
    }
    return GetSessionFromSessionMap(uuid);
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


MessagePtr __YRPC_SessionManager::Handler_HandShake(const MessagePtr msg,SessionPtr sess)
{
    auto req = std::static_pointer_cast<C2S_HANDSHAKE_REQ>(msg);
    req->connector_ip();
    req->connector_port();
    auto peer_addr = Address(sess->GetPeerAddress().GetIP(), req->connector_port());
    auto peer_uuid = bbt::uuid::UuidMgr::CreateUUid(req->uuid());
    {
        lock_guard<Mutex> lock(m_mutex_session_map);
        // 保存到 knownode map  -- todo 优化对端数据
        auto [_, succ] = m_knownode_map.insert(std::make_pair(peer_addr, peer_uuid));
        // 保存到 session map
        if( nullptr == Append_SessionMap(peer_uuid, sess) )
        {
            ERROR("[YRPC][__YRPC_SessionManager::Handler_HandShake][%d] session map repeat key! ip: {%s}", y_scheduler_id, sess->GetPeerAddress().GetIPPort());
            sess->Close();
        }
    }
    // 握手成功，设置rsp
    auto rsp = std::make_shared<S2C_HANDSHAKE_RSP>();
    rsp->set_uuid(m_local_node_id->GetRawString());
    rsp->set_acceptor_ip(m_local_addr.GetIP());
    rsp->set_acceptor_port(m_local_addr.GetPort());
    // 关闭定时器
    sess->StopHandShakeTimer();
    // 握手成功回调
    errorcode err(
        "handshake ok!", 
        yrpc::detail::shared::YRPC_ERR_TYPE::ERRTYPE_HANDSHAKE, 
        yrpc::detail::shared::ERR_HANDSHAKE_SUCCESS);
    OnHandShakeFinal(err, sess);
    return rsp;
}

void __YRPC_SessionManager::StartHandShake(const yrpc::detail::shared::errorcode& e, SessionPtr sess)
{
    if( e.err() == yrpc::detail::shared::ERR_NETWORK_CONN_OK )
    {
        C2S_HANDSHAKE_REQ req;
        req.set_uuid(m_local_node_id->GetRawString());
        req.set_connector_ip(m_local_addr.GetIP());
        req.set_connector_port(m_local_addr.GetPort());
        auto callobj = CallObjFactory::GetInstance()->Create<C2S_HANDSHAKE_REQ, S2C_HANDSHAKE_RSP>(std::move(req), "YRPC_HandShake", 
                [=](MessagePtr rsp){ HandShakeRsp(rsp, sess); });
        if( 0 > sess->SendACallObj(callobj) )
        {
            ERROR("[YRPC][__YRPC_SessionManager::StartHandShake][%d] send handshake failed!", y_scheduler_id);
        }
        sess->StartHandShakeTimer([this](const errorcode& e, SessionPtr sess){
            OnHandShakeTimeOut(e, sess);
        },3000);
    }
    else
    {
        FATAL("[YRPC][__YRPC_SessionManager::StartHandShake][%d] fatal! maybe coroutine error!", y_scheduler_id);
    }
}

void __YRPC_SessionManager::OnHandShakeFinal(const yrpc::detail::shared::errorcode& e, SessionPtr sess)
{
    INFO("[YRPC][__YRPC_SessionManager::OnHandShakeFinal][%d] handshake success!", y_scheduler_id);
}

void __YRPC_SessionManager::OnHandShakeTimeOut(const yrpc::detail::shared::errorcode& e, SessionPtr sess)
{
    if( e.err() == yrpc::detail::shared::ERR_HANDSHAKE_TIMEOUT )
    {
        auto [handshakedata, exist] = m_undone_conn_queue->PopUpById(sess->GetPeerAddress());
        errorcode err;
        err.settype(yrpc::detail::shared::ERRTYPE_HANDSHAKE);
        err.setcode(yrpc::detail::shared::ERR_HANDSHAKE_TIMEOUT);
        if( !exist )
        {
            err.setcode(yrpc::detail::shared::ERR_HANDSHAKE_SESS_NOTEXIST);
            FATAL("[YRPC][__YRPC_SessionManager::OnHandShakeTimeOut][%d] undone queue order timing error!", y_scheduler_id);
        }
        sess->Close();
        handshakedata.m_succ(err, nullptr); // 握手完成回调通知
    }
    else
    {
        ERROR("[YRPC][__YRPC_SessionManager::OnHandShakeTimeOut][%d] other errors!", y_scheduler_id);
    }
}

void __YRPC_SessionManager::HandShakeRsp(MessagePtr msg, SessionPtr sess)
{
    /* 处理握手响应 */
    auto rsp = std::static_pointer_cast<S2C_HANDSHAKE_RSP>(msg);
    auto peer_uuid = bbt::uuid::UuidMgr::CreateUUid(rsp->uuid());
    auto peer_addr = Address(sess->GetPeerAddress().GetIP(), sess->GetPeerAddress().GetPort());
    errorcode err("",
            yrpc::detail::shared::ERRTYPE_HANDSHAKE,
            yrpc::detail::shared::ERR_HANDSHAKE_UNDONE_FAILED);
    sess->SetPeerUuid(peer_uuid);
    SessionPtr result_ptr;
    std::pair<HandShakeData, bool>  it_undone_sess;
    {
        lock_guard<Mutex> lock(m_mutex_session_map);
        // 从半连接队列总取出session
        it_undone_sess = m_undone_conn_queue->PopUpById(sess->GetPeerAddress());
        if( !it_undone_sess.second )
        {
            ERROR("[YRPC][__YRPC_SessionManager::HandShakeRsp][%d] {%s} not found in undone session map!", y_scheduler_id, peer_addr.GetIPPort().c_str());
            return;
        }
        // 插入到 session map 中
        result_ptr = Append_SessionMap(peer_uuid, it_undone_sess.first.m_sess);
    }
    auto&& sess_data = it_undone_sess.first;
    // 添加失败
    if( result_ptr == nullptr )
    {
        err.setcode(yrpc::detail::shared::ERR_HANDSHAKE_SESS_EXIST);
        sess_data.m_succ(err, sess_data.m_sess);
        sess_data.m_sess->Close();
        INFO("[YRPC][__YRPC_SessionManager::HandShakeRsp][%d] handshake error!", y_scheduler_id);
        return;
    }
    err.setcode(yrpc::detail::shared::ERR_HANDSHAKE_SUCCESS);
    sess_data.m_succ(err, sess_data.m_sess);
}

SessionPtr __YRPC_SessionManager::InitRpcSession(Channel::SPtr new_chan)
{
    auto session_ptr = RpcSession::Create(new_chan);
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

void __YRPC_SessionManager::OnSessionTimeOut(const yrpc::detail::shared::errorcode& e, SessionPtr addr)
{
    // todo
}

void __YRPC_SessionManager::OnSessionClose(const yrpc::detail::shared::errorcode& e, SessionPtr sess)
{
    {
        lock_guard<Mutex> lock(m_mutex_session_map);
        auto peer_sess = Delete_SessionMap(sess->GetPeerUuid());
        if( nullptr == peer_sess )
        {
            ERROR("[YRPC][__YRPC_SessionManager::OnSessionClose][%d] ", y_scheduler_id);
        }
    }
    // DelSession(sess->GetPeerUuid());
    DEBUG("[YRPC][__YRPC_SessionManager::OnSessionClose][%d] RpcSession closed! delete info from SessionMgr!", y_scheduler_id);
}

bbt::uuid::UuidBase::Ptr __YRPC_SessionManager::GetUuid(const Address& key)
{
    // lock_guard<Mutex> lock(m_mutex_session_map);
    auto it = m_knownode_map.find(key);
    if( it == m_knownode_map.end() )
        return nullptr;
    return it->second;
}

#undef BalanceNext