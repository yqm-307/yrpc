#include "SessionManager.h"
#include <unistd.h>
using namespace yrpc::rpc::detail;


SessionManager::SessionManager(int Nthread)
    :port(7912),
    m_loop_latch(Nthread-1),
    m_main_loop(new Epoller(64*1024,65535)),
    m_main_acceptor(m_main_loop,8121,2000,1000),
    m_connector(m_main_loop)
{
    // 初始化 main eventloop，但是不运行
    m_main_loop->RunForever();
    m_main_loop->AddTask([this](void*){ RunInMainLoop(); },nullptr);            // 注册任务
    m_main_thread = new std::thread([this](){this->m_main_loop->Loop();});      // 线程运行

    // 初始化 sub eventloop，并运行（由于队列为空，都挂起）
    assert(Nthread>=2);
    m_sub_loop = new Epoller*[Nthread-1];
    m_sub_threads = new std::thread*[Nthread-1];
    for (int i=1;i<Nthread;++i)
    {
        auto tmp = m_sub_loop[i-1];
        tmp = new Epoller(64*1024,4096);
        tmp->RunForever();
        tmp->AddTask([this](void* ep){
            this->RunInSubLoop((Epoller*)ep);
        },tmp);
        m_sub_threads[i-1] = new std::thread([this,tmp](){
            this->m_loop_latch.wait();
            tmp->Loop();
        });
        m_loop_latch.down();
    }

    INFO("%s:%s , info: SessionManager Init Success!",__FUNCTION__,__LINE__);
    
}



void SessionManager::AddNewSession(Address addr,RpcSession* session)
{
    // lock_guard<Mutex> lock(m_mutex);
    if (  )
}

bool SessionManager::DelSession(SessionID id)
{

}


void SessionManager::RunInMainLoop()
{
    m_main_acceptor.listen();
}

void SessionManager::RunInSubLoop(Epoller* lp)
{
    lp->Loop();
}

void SessionManager::OnAccept(ConnPtr newconn,void* ep)
{
    auto loop = (Epoller*)ep;
    // todo 新连接建立session保存在manager
    if (newconn == nullptr)
        return;
    auto channel = Channel::Create(newconn);
    yrpc::rpc::detail::RpcSession* newsession = new RpcSession(channel,loop);

    // SessionID newid = m_hash((intptr_t)newsession); 
    // 添加到数据结构中
    m_sessions.insert(std::make_pair(m_id_key.load(),new RpcSession(channel,loop)));
    m_id_key++;


}



SessionManager *SessionManager::GetInstance(int n)
{
    static SessionManager *manager = nullptr;
    if (manager == nullptr)
        if(n == 0)
            manager = new SessionManager(sysconf(_SC_NPROCESSORS_ONLN)*2);  // 默认根据处理器数量*2 分配线程
    return manager;
}


void SessionManager::AsyncConnect(Address peer,OnSession onsession)
{
    using namespace yrpc::detail::net;
    decltype(m_addrtoid)::iterator it = m_addrtoid.find(peer.GetIPPort());
    // 连接是否已经存在
    if( it == m_addrtoid.end() )
    {   // 不存在，新建连接
        RoutineSocket* socket = Connector::CreateSocket();
        m_connector.AsyncConnect(socket,peer,[this](const errorcode& e,const ConnectionPtr& conn,void* ){

        }); // 新建连接
    }
    else
    {
        auto sess = m_sessions.find(it->second);
        assert(sess != m_sessions.end());
        onsession(sess->second);
    }
}


void SessionManager::OnConnect(ConnPtr conn)
{

}
