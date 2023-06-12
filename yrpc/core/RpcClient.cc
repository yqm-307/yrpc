#include "SessionManager.h"
#include "RpcClient.h"
#include <algorithm>

namespace yrpc::rpc
{



RpcClient::RpcClient(std::string ip,int port)
    :m_session(nullptr),
    m_addr(ip,port)
{
}

RpcClient::RpcClient(yrpc::detail::net::YAddress servaddr_)
    :m_session(nullptr),
    m_addr(servaddr_)
{
}



RpcClient::~RpcClient()
{
    m_callmap.clear();
    
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
        m_session->SetToClientCallback(functor([this](Buffer&& pck,detail::SessionPtr){OnPckHandler(std::move(pck));}));
    }
    else
    {
        m_session = nullptr;
    }
}


void RpcClient::OnPckHandler(Buffer&&/*字节流*/ pck)
{
    Resolver rsl(pck);
    // 获取包id,很重要，to select callobjmap
    {
        yrpc::util::lock::lock_guard<Mutex> lock(m_mutex);
        auto it = m_callmap.find(rsl.GetProtoID());
        if (it == m_callmap.end())
        {
            ERROR("RpcClient::OnPckHandler() , info: cann`t find package id");
        }
        else
        {
            it->second->SetResult(rsl);     //设置结果
            m_callmap.erase(it);
        }
    }
}




int RpcClient::Call(detail::CallObj::Ptr call)
{
    int ret = 0;

    do{
        if (m_session == nullptr)
        {
            ERROR("session is not connected!");
            ret = -1;
            break;
        }
        if (m_session->IsClosed())
        {
            ERROR("session is closed!");
            ret = -1;
            break;
        }
        {
            yrpc::util::lock::lock_guard<Mutex> lock(m_mutex);
            int id = call->GetID();
            auto res = m_callmap.insert(std::make_pair(id, call));
            // auto res = m_callmap.insert(std::make_pair(call->GetID(), call));
            if (!res.second)
            {
                ERROR("call object ID is repeat!");
                ret = -1;
                break;
            } // 相同值

            ret = m_session->Append(call->m_req);
        }

    }while(0);

    return ret;
}

/**
 * @brief 
 * 
 * @param func 
 */
void RpcClient::AsyncConnect(detail::OnConnCallBack userfunc)
{
    yrpc::rpc::detail::SessionManager::GetInstance()->AsyncConnect(m_addr,[this,userfunc](SessionPtr ptr){
        this->OnConnect(ptr);
        if (userfunc != nullptr)
        {
            userfunc();
        }
    });
}

}