#include "servicemap.h"
#include <assert.h>
#include <algorithm>
#include <iostream>
#include "../Util/logger.h"
#include "../Util/BKDRHash.h"

namespace yrpc::detail
{

ServiceMap::ServiceMap()
{

}

ServiceMap::~ServiceMap()
{
    //全部析构掉
    for(auto it : m_id2service)
    {
        delete &it;
    }
}

uint32_t ServiceMap::insert(std::string name,const ServiceFunc& service,const CodecFunc& code)
{
    if(m_name2id.find(name) != m_name2id.end())
    {//服务名冲突
        FATAL("[YRPC][ServiceMap::insert] service name repeated!");
        return 0;
    }

    auto ret = yrpc::util::hash::BKDRHash(name.c_str(),name.size());
    assert(m_id2name.find(ret) == m_id2name.end());     //hash冲突,需要修改名字
    ServiceHandles* ptr = new ServiceHandles{service,code};
    m_id2service.insert({ret,ptr});
    m_name2id.insert({name,ret});
    m_id2name.insert({ret,name});
    return ret;
}

uint32_t ServiceMap::insert(std::string name, uint32_t id,const ServiceFunc& service ,const CodecFunc& code)
{
    if(m_name2id.find(name) != m_name2id.end())
    {
        FATAL("[YRPC][ServiceMap::insert] service name repeated!");
        return 0;
    }

    auto ret = id;
    assert(m_id2name.find(ret) == m_id2name.end());
    ServiceHandles* ptr = new ServiceHandles{service,code};
    m_id2service.insert({ret,ptr});
    m_name2id.insert({name,ret});
    m_id2name.insert({ret,name});
    return ret;
}


int ServiceMap::earse(int id)
{
    if(m_id2service.size() <= 0)
        return -1;
    else
    {
        if(m_id2service.erase(id))
            return 0;
        else
            return -1;
    }
    return -1;
}


const ServiceHandles* ServiceMap::IdToService(uint32_t id) const 
{
    if(m_id2service.size() <= 0)
        return nullptr;
    else
    {
        auto it = m_id2service.find(id);
        if(it == m_id2service.end())
            return nullptr;
        else
            return it->second;
    }
    return nullptr;
}


const ServiceHandles* ServiceMap::NameToService(std::string name) const
{
    if(m_name2id.size() <=0)
        return nullptr;
    else
    {
        auto it = m_name2id.find(name);
        if(it == m_name2id.end())
            return nullptr;
        else
            return IdToService(it->second);
    }
    return nullptr;
}



}