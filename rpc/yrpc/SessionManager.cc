#include "SessionManager.h"
#include <unistd.h>
using namespace yrpc::rpc::detail;


SessionManager::SessionManager(int Nthread)
    :port(7912)
{
    // 初始化 main eventloop，但是不运行

    // 初始化 sub eventloop，并运行（由于队列为空，都挂起）

    // 等待 sub eventloop 启动完毕（运行main）

    // 初始化完毕
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
