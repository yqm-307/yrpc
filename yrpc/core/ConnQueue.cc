#include "ConnQueue.hpp"



using namespace yrpc::rpc::detail;


std::pair<HandShakeData,bool> ConnQueue::PopUpById(const Address& addr)
{
    auto it = m_undone_rpc_session_map.find(addr);
    if(it == m_undone_rpc_session_map.end())
        return {{},false};
    else
    {
        m_undone_rpc_session_map.erase(it);
        return {it->second,true};
    }
}

int ConnQueue::FindAndPush(const Address& addr, const HandShakeData& func)
{
    int ret = -1;
    auto it = m_undone_rpc_session_map.find(addr);
    if (it != m_undone_rpc_session_map.end())
    {
    }
    else
    {
        m_undone_rpc_session_map.insert(std::make_pair(addr, func));
        ret = 1;
    }
    return ret;
}

const HandShakeData* ConnQueue::Find(const Address& id)
{
    auto it = m_undone_rpc_session_map.find(id);
    if (it == m_undone_rpc_session_map.end())
        return nullptr;
    else
        return &(it->second);  
}


bool ConnQueue::HasWaitting(const Address& addr)
{
    auto it_tcp = m_undone_tcp_conn_map.find(addr);
    if( it_tcp != m_undone_tcp_conn_map.end() )
    {
        return true;
    }
    auto it_sess = m_undone_rpc_session_map.find(addr);
    if( it_sess != m_undone_rpc_session_map.end() )
    {
        return true;
    }
    return false;
}

bool ConnQueue::AddTcpConn(const Address& addr)
{
    auto [_,suss] = m_undone_tcp_conn_map.insert(addr);
    return suss;
}

bool ConnQueue::HasTcpConn(const Address& addr)
{
    auto it = m_undone_tcp_conn_map.find(addr);
    return (it != m_undone_tcp_conn_map.end());
}

bool ConnQueue::DelTcpConn(const Address& addr)
{
    auto earse_num = m_undone_tcp_conn_map.erase(addr);
    return (earse_num == 1);
}