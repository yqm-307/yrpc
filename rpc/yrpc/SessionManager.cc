#include "SessionManager.h"

using namespace yrpc::rpc::detail;


SessionManager::SessionManager()
    :port(7912)
{

}



SessionManager *SessionManager::GetInstance()
{
    static SessionManager *manager = nullptr;
    if (manager == nullptr)
        manager = new SessionManager();
    return manager;
}

// 创建session 然后连接对端。
int SessionManager::CreateNewSession() // 创建一个session
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
