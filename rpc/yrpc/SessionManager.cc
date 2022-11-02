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



SessionManager::SessionPtr SessionManager::AddNewSession(Channel::ChannelPtr ptr)
{
    lock_guard<Mutex> lock(m_mutex_sessions);
    auto sessionptr = RpcSession::Create(ptr,m_sub_loop[BalanceNext]);
    // 创建并初始化新Session
    sessionptr->SetCloseFunc([this](const yrpc::detail::shared::errorcode& e,const yrpc::detail::net::YAddress& addr){
        // 连接断开，从SessionMap中删除此Session
        this->DelSession(addr);
    });
    m_sessions.insert(std::make_pair(AddressToID(ptr->GetConnInfo()->GetPeerAddress()),sessionptr));        // session 映射
    return sessionptr;
}

bool SessionManager::DelSession(const Address& id)
{    
    lock_guard<Mutex> lock(m_mutex_sessions);
    auto its = m_sessions.find(AddressToID(id));
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

void SessionManager::OnAccept(Channel::ChannelPtr newchannel,void* ep)
{
    /**
     * 被连接后,注册到SessionMap中
     */
    AddNewSession(newchannel);
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
    
    static std::map<SessionID,std::queue<OnSession>> _connect_async_wait_queue;
    static Mutex _lock;
    
    SessionID id = AddressToID(peer);
    auto it = m_sessions.find(id);

    // auto it = m_addrtoid.find(peer.GetIPPort());
    if( it == m_sessions.end() )
    {   // 尚未建立完成，有两种可能:1、已经开始connect;2、尚未开始connect

        lock_guard<Mutex> lock(_lock);
        auto iter = _connect_async_wait_queue.find(id);
        if(iter == _connect_async_wait_queue.end())
        {// 第二种情况
            std::queue<OnSession> queue;
            queue.push(onsession);
            _connect_async_wait_queue.insert(std::make_pair(id,queue));

            // 初始化套接字，并注册回调。回调需要在完成连接之后，清除连接等待队列、更新SessionMap   &_connect_async_wait_queue,&_lock,
            RoutineSocket* socket = Connector::CreateSocket();

            // 回调
            m_connector.AsyncConnect(socket,peer,[this](const errorcode& e,const ConnectionPtr& conn){
                // 构造新的Session
                if(e.err() == yrpc::detail::shared::ERR_NETWORK_CONN_OK)
                {
                    auto channelptr = Channel::Create(conn);    // 建立通信信道
                    // 更新SessionMap
                    auto newSession = this->AddNewSession(channelptr);
                    {// clean 连接等待队列，处理回调任务
                        auto it = _connect_async_wait_queue.find(AddressToID(conn->GetPeerAddress()));
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

        auto it_sess = m_sessions.find(AddressToID(id));
        if (it_sess == m_sessions.end())
        {   // 注册了，但是尚未连接成功
            lock_guard<Mutex> lock(_lock);
            auto iter = _connect_async_wait_queue.find(id);
            iter->second.push(onsession);
        }
        else
        {   // 注册了，且Session已经在了，直接返回
            onsession(it_sess->second);
        }
    }

    {// 其实可以抽出来写个函数的

    }


}

SessionManager::SessionID SessionManager::AddressToID(const Address& addr)
{
    auto str = addr.GetIPPort();
    std::string id(19,'0');
    int j=0;
    for(int i =0;i<str.size();++i)
    {
        if(str[i] >= '0' || str[i] <= '9')
        {
            id[j++] = str[i];
        }
    }
    return std::stoull(id);
}


void SessionManager::OnConnect(Channel::ChannelPtr conn)
{

}













#undef BalanceNext