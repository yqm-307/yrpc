#include "SessionManager.h"
#include <unistd.h>
using namespace yrpc::rpc::detail;


SessionManager::SessionManager(int Nthread)
    :port(7912),
    m_loop_latch(Nthread-1),
    m_main_loop(new Epoller(64*1024,65535)),
    m_main_acceptor(m_main_loop,8121,2000,1000)
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




void SessionManager::RunInMainLoop()
{
    m_main_acceptor.listen();
}

void SessionManager::RunInSubLoop(Epoller* lp)
{
    lp->Loop();
}



SessionManager *SessionManager::GetInstance(int n)
{
    static SessionManager *manager = nullptr;
    if (manager == nullptr)
        if(n == 0)
            manager = new SessionManager(sysconf(_SC_NPROCESSORS_ONLN)*2);  // 默认根据处理器数量*2 分配线程
    return manager;
}

// 创建session 然后连接对端。
int SessionManager::CreateNewSession(int Nthread) // 创建一个session
{

}


int SessionManager::Submit(const yrpc::util::buffer::Buffer& buff,SessionID session)
{
    auto it = m_client_sessions.find(session);
    if( it == m_client_sessions.end() )
    {//连接不存在，建立连接
        // connector_.connect();
        // ERROR("SessionManager::Submit_Call() , info: ");
    }
    else
    {//连接已经存在，直接提交
        // it->second->RpcAsyncCall(buff);
    }
}


SessionManager::SessionID SessionManager::GetSessionID()
{
    static uint32_t count=1;
    return count++;
}
