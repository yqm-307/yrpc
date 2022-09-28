#include "SessionManager.h"

using namespace yrpc::rpc::detail;

SessionManager *SessionManager::GetInstance()
{
    static SessionManager *manager = nullptr;
    if (manager == nullptr)
        manager = new SessionManager();
    return manager;
}

// 创建session 然后连接对端。
int SessionManager::Make_Session() // 创建一个session
{

}


int SessionManager::Submit_Call(const yrpc::util::buffer::Buffer& buff,const yrpc::detail::ynet::YAddress& addr)
{
    auto it = m_client_sessions.find(addr);
    if( it == m_client_sessions.end() )
    {//连接不存在，建立连接
        connector_.connect();
        // ERROR("SessionManager::Submit_Call() , info: ");
    }
}

