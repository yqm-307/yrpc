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
        assert(m_session != nullptr);
        m_session->SetToClientCallback([this](std::string& pck){OnPckHandler(pck);});
    }
    else
    {
        m_session = nullptr;
    }
}


void RpcClient::OnPckHandler(std::string&/*字节流*/ pck)
{
    Resolver rsl(pck);
    // 获取包id,很重要，to select callobjmap
    auto it = m_callmap.find(rsl.GetProtoID());
    if (it == m_callmap.end())
    {
        ERROR("RpcClient::OnPckHandler() , info: cann`t find package id");
    }
    

}


int CallObj()
{

}

int GetResult()
{

}


}