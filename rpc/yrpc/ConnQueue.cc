#include "ConnQueue.hpp"



using namespace yrpc::rpc::detail;


ConnQueue::OnSessionCallback ConnQueue::PopUpById(SessionID id)
{
    auto it = m_map.find(id);
    if(it == m_map.end())
        return nullptr;
    else
        return it->second;
}

int ConnQueue::FindAndPush(SessionID id, const OnSessionCallback& func)
{
    int ret = -1;
    auto it = m_map.find(id);
    if (it != m_map.end())
    {
        ret = 1; // 已经存在
    }
    else
    {
        auto succ =  m_map.insert(std::make_pair(id, func));
        ret = succ.second ? 2 : -2; 
    }
    return ret;
}

ConnQueue::OnSessionCallback ConnQueue::Find(SessionID id)
{
    return m_map.find(id)->second;  
}