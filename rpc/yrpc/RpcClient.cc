#include "RpcClient.h"


namespace yrpc::rpc
{




RpcClient::RpcClient(std::string ip,int port)
    :m_addr(ip,port),
    m_session(nullptr)
{
    SessionManager::GetInstance()->AsyncConnect(m_addr,[this](SessionPtr ptr){
        this->OnConnect(ptr);
    });

}



bool RpcClient::IsConnected()
{
    if (m_session != nullptr && !m_session->IsClosed())
        return true;
    else
        return false;
}

void RpcClient::OnConnect(SessionPtr newsession)
{
    if (newsession != nullptr)
    {
        m_session = newsession;
    }
    else
    {
        m_session = nullptr;
    }
}


int RpcClient::AsyncCall()
{

}


}