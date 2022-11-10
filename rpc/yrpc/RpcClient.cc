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

RpcClient::RpcClient(yrpc::detail::net::YAddress servaddr_)
    :m_addr(servaddr_),
    m_session(nullptr)
{
}



RpcClient::~RpcClient()
{
    
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
    assert(this);
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
    it->second->SetResult(rsl);     //设置结果
}




int RpcClient::Call(detail::CallObj::Ptr&& call)
{
    yrpc::util::lock::lock_guard<Mutex> lock(m_mutex);
    auto res = m_callmap.insert(std::make_pair(call->GetID(),call));
    if (!res.second)    // 相同值
        return -1;
    std::string bytes;
    call->GetRequest().ToByteArray(bytes);
    m_session->Append(bytes);
}
}