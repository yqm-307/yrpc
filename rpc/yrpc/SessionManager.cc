#include "SessionManager.h"
#include <unistd.h>
using namespace yrpc::rpc::detail;

#define BalanceNext ( m_balance = (m_balance + 1) % (m_sub_loop_size))


__YRPC_SessionManager::__YRPC_SessionManager(int Nthread)
    :m_main_loop(new Epoller(64*1024,65535)),
    m_main_acceptor(nullptr),
    m_connector(m_main_loop),
    m_sub_loop_size(Nthread-1),
    m_loop_latch(Nthread-1),
    m_conn_queue(std::make_unique<ConnectWaitQueue_Impl>()),
    port(7912)
{
    // 初始化 main eventloop，但是不运行
    m_main_thread = new std::thread([this](){
        m_main_loop->RunForever();
        this->m_main_loop->Loop();
    });      // 线程运行

    // 初始化 sub eventloop，并运行（由于队列为空，都挂起）
    assert(Nthread>=2);
    m_sub_loop = new Epoller*[Nthread-1];
    m_sub_threads = new std::thread*[Nthread-1];
    for (int i=0;i<Nthread;++i)
    {
        m_sub_loop[i] = new Epoller(64*1024,4096);
        auto& tmp = m_sub_loop[i];
        tmp->RunForever();
        tmp->AddTask([this](void* ep){
            this->RunInSubLoop((Epoller*)ep);
        },tmp);
        m_sub_threads[i] = new std::thread([this,tmp](){
            this->m_loop_latch.wait();
            tmp->Loop();
        });
        m_loop_latch.down();
    }

    INFO("__YRPC_SessionManager()  , info: SessionManager Init Success!");
    
}



__YRPC_SessionManager::SessionPtr __YRPC_SessionManager::AddNewSession(Channel::ConnPtr connptr)
{
    /**
     * 建立新连接
     * 
     * 建立新连接，并保存在Manager中，注意注册超时、关闭时回调
     */
    lock_guard<Mutex> lock(m_mutex_sessions);
    auto nextloop = m_sub_loop[BalanceNext];
    auto channelptr = Channel::Create(connptr,nextloop);
    auto sessionptr = RpcSession::Create(channelptr,*m_sub_loop+BalanceNext);
    // 创建并初始化新Session
    sessionptr->SetCloseFunc([this](const yrpc::detail::shared::errorcode& e,const yrpc::detail::net::YAddress& addr){
        // 连接断开，从SessionMap中删除此Session
        this->DelSession(addr);
    });

    sessionptr->SetTimeOutFunc([sessionptr](){
        sessionptr->Close();    // 触发 CloseCallback
    });

    m_sessions.insert(std::make_pair(AddressToID(channelptr->GetConnInfo()->GetPeerAddress()),sessionptr));        // session 映射
    return sessionptr;
}

bool __YRPC_SessionManager::DelSession(const YAddress& id)
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



__YRPC_SessionManager *__YRPC_SessionManager::GetInstance(int n)
{
    static __YRPC_SessionManager *manager = nullptr;
    if (manager == nullptr)
        if(n == 0)
            manager = new __YRPC_SessionManager(sysconf(_SC_NPROCESSORS_ONLN)*2);  // 默认根据处理器数量*2 分配线程
    return manager;
}


bool __YRPC_SessionManager::AsyncConnect(YAddress peer,OnSession onsession)
{
    using namespace yrpc::detail::net;




    bool ret = false;    
    SessionID id = AddressToID(peer);
    auto it = m_sessions.find(id);

    /**
     * 1、如果sessions中已经有了该连接，则直接返回即可
     * 2、如果sessions中没有，则检查是否已经注册连接事件
     *      (1) 如果 connect async wait queue 中没有对应的用户会话，注册一个新的connect事件
     *      (2) 如果 connect async wait queue 中有对应的用户会话，返回false
     */
    if( it == m_sessions.end() )
    {
        lock_guard<Mutex> lock(m_conn_queue->m_mutex);
        Socket* socket = Connector::CreateSocket();
        auto id = m_conn_queue->FindAndInsert(peer,onsession);
        if( id < 0 )
        {
            ret = false;
        }
        else
        {
            m_connector.AsyncConnect(socket, peer, functor([this](const errorcode &e, ConnectionPtr conn)
            {
                // 构造新的Session
                if(e.err() == yrpc::detail::shared::ERR_NETWORK_CONN_OK)
                {
                    // 更新SessionMap
                    auto newSession = this->AddNewSession(conn);
                    {// clean 连接等待队列，处理回调任务
                        lock_guard<Mutex> lock(m_conn_queue->m_mutex);
                        auto onsessfunc = this->m_conn_queue->FindAndRemove(conn->GetPeerAddress());
                        if(onsessfunc != nullptr)
                           onsessfunc(newSession);
                        else
                            ERROR("cannot find register func");
                    }
                }
                else
                {
                    /* todo 网络连接失败错误处理 */
                } 
            }));
            ret = true;
        }
    }
    else
    {
        onsession(it->second);
        ret = true;
    }

    return ret;
}

__YRPC_SessionManager::SessionID __YRPC_SessionManager::AddressToID(const YAddress& addr)
{
    auto str = addr.GetIPPort();
    std::string id(19,'0');
    int j=0;
    for(int i =0;i<str.size();++i)
    {
        if(str[i] >= '0' && str[i] <= '9')
        {
            id[j++] = str[i];
        }
    }
    return std::stoull(id);
}


void __YRPC_SessionManager::OnConnect(Channel::ChannelPtr conn)
{

}




void __YRPC_SessionManager::AsyncAccept(YAddress peer,RpcSession::DispatchCallback dspcb)
{
    assert(!m_server.IsServiceSupplier());   // 不允许重注册！
    m_server.Init(dspcb);
    if(m_main_acceptor != nullptr)
        delete m_main_acceptor;
    m_main_acceptor = new Acceptor(m_main_loop,peer.GetPort(),3000,5000);
    // m_main_acceptor->setOnConnect([](){});
    m_main_loop->AddTask([this](void*){this->RunInMainLoop();});

}









#undef BalanceNext