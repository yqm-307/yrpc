#include "ConnQueue.hpp"



using namespace yrpc::rpc::detail;


std::pair<HandShakeData,bool> ConnQueue::PopUpById(const Address& addr)
{
    auto it = m_map.find(addr);
    if(it == m_map.end())
        return {{},false};
    else
    {
        m_map.erase(it);
        return {it->second,true};
    }
}

int ConnQueue::FindAndPush(const Address& addr, const HandShakeData& func)
{
    int ret = -1;
    auto it = m_map.find(addr);
    if (it != m_map.end())
    {
    }
    else
    {
        m_map.insert(std::make_pair(addr, func));
        ret = 1;
    }
    return ret;
}

const HandShakeData* ConnQueue::Find(SessionID id)
{
    auto it = m_map.find(id);
    if (it == m_map.end())
        return nullptr;
    else
        return &(it->second);  
}
