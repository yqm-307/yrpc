#include "SessionManager.h"
#include <unistd.h>
using namespace yrpc::rpc::detail;

#define BalanceNext ( m_balance = (m_balance + 1) % (m_sub_loop_size))


SessionManager::SessionManager(int Nthread)
    :port(7912),
    m_sub_loop_size(Nthread-1),
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



SessionManager::SessionPtr SessionManager::AddNewSession(const Address& addr,Channel::ChannelPtr ptr)
{
    lock_guard<Mutex> lock(m_mutex_sessions);
    
    auto sessionptr = std::make_shared<RpcSession>(ptr,m_sub_loop[BalanceNext]);
    auto it = m_addrtoid.find(addr.GetIPPort());
    m_sessions.insert(std::make_pair(it->second,sessionptr));        // session 映射
    return sessionptr;
}

bool SessionManager::DelSession(const Address& id)
{
    lock_guard<Mutex> lock(m_mutex_addrid);
    auto it = m_addrtoid.find(id.GetIPPort());
    m_addrtoid.erase(it);
    
    auto its = m_sessions.find(it->second);
    if ( its != m_sessions.end() )
    {
        m_sessions.erase(its);
        return true;
    }
    else
        return false;   // 不存在此节点
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
    
    static std::map<std::string,std::queue<OnSession>> _connect_async_wait_queue;
    static Mutex _lock;
    
    auto addr_peer = peer.GetIPPort();
    auto it = m_addrtoid.find(peer.GetIPPort());
    if( it == m_addrtoid.end() )
    {   // 不存在，新建连接

        lock_guard<Mutex> lock(_lock);
        auto iter = _connect_async_wait_queue.find(addr_peer);
        if(iter == _connect_async_wait_queue.end())
        {// 连接尚未开始
            m_addrtoid.insert(std::make_pair(addr_peer,GetNewID()));
            std::queue<OnSession> queue;
            queue.push(onsession);
            _connect_async_wait_queue.insert(std::make_pair(addr_peer,queue));

            // 初始化套接字，并注册回调。回调需要在完成连接之后，清除连接等待队列、更新SessionMap   &_connect_async_wait_queue,&_lock,
            RoutineSocket* socket = Connector::CreateSocket();
            m_connector.AsyncConnect(socket,peer,[this](const errorcode& e,const ConnectionPtr& conn,void* ep){
                // 构造新的Session
                if(e.err() == yrpc::detail::shared::ERR_NETWORK_CONN_OK)
                {
                    auto channelptr = Channel::Create(conn);    // 建立通信信道
                    // 更新SessionMap
                    auto newSession = this->AddNewSession(conn->GetPeerAddress(),channelptr);
                    {// clean 连接等待队列
                        auto it = _connect_async_wait_queue.find(conn->StrIPPort());
                        if (it == _connect_async_wait_queue.end())
                        {
                            DEBUG("线程安全问题");
                        }
                        auto conncb_queue = _connect_async_wait_queue.erase(it)->second;
                        while(!conncb_queue.empty())    // 回调通知
                        {
                            conncb_queue.front().operator()(newSession);
                        }
                    }
                }
                else
                {
                    /* todo 错误处理 */
                }
            });
        }
        else
        {// 连接正在进行中
            iter->second.push(onsession);
        }
        return;


        
    }
    else
    {   // 起码已经注册了，但不一定已经建立连接

        auto it_sess = m_sessions.find(it->second);
        if (it_sess == m_sessions.end())
        {   // 注册了，但是尚未连接成功
            lock_guard<Mutex> lock(_lock);
            auto iter = _connect_async_wait_queue.find(addr_peer);
            iter->second.push(onsession);
        }
        else
        {   // 注册了，且Session已经在了
            onsession(it_sess->second);
        }


    }

    {// 其实可以抽出来写个函数的

    }


}



void SessionManager::OnConnect(ConnPtr conn)
{

}













#undef BalanceNext