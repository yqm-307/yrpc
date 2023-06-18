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
    for(auto it : IdToService_)
    {
        delete &it;
    }
}

uint32_t ServiceMap::insert(std::string name,const ServiceFunc& service,const CodecFunc& code)
{
    if(NameToId_.find(name) != NameToId_.end())
    {//服务名冲突
        FATAL("[YRPC][ServiceMap::insert] service name repeated!");
        return 0;
    }

    auto ret = yrpc::util::hash::BKDRHash(name.c_str(),name.size());
    assert(IdToName_.find(ret) == IdToName_.end());     //hash冲突,需要修改名字
    ServiceHandles* ptr = new ServiceHandles{service,code};
    IdToService_.insert({ret,ptr});
    NameToId_.insert({name,ret});
    IdToName_.insert({ret,name});
    return ret;
}

uint32_t ServiceMap::insert(std::string name, uint32_t id,const ServiceFunc& service ,const CodecFunc& code)
{
    if(NameToId_.find(name) != NameToId_.end())
    {
        FATAL("[YRPC][ServiceMap::insert] service name repeated!");
        return 0;
    }

    auto ret = id;
    assert(IdToName_.find(ret) == IdToName_.end());
    ServiceHandles* ptr = new ServiceHandles{service,code};
    IdToService_.insert({ret,ptr});
    NameToId_.insert({name,ret});
    IdToName_.insert({ret,name});
    return ret;
}


int ServiceMap::earse(int id)
{
    if(IdToService_.size() <= 0)
        return -1;
    else
    {
        if(IdToService_.erase(id))
            return 0;
        else
            return -1;
    }
    return -1;
}


const ServiceHandles* ServiceMap::IdToService(uint32_t id) const 
{
    if(IdToService_.size() <= 0)
        return nullptr;
    else
    {
        auto it = IdToService_.find(id);
        if(it == IdToService_.end())
            return nullptr;
        else
            return it->second;
    }
    return nullptr;
}


const ServiceHandles* ServiceMap::NameToService(std::string name) const
{
    if(NameToId_.size() <=0)
        return nullptr;
    else
    {
        auto it = NameToId_.find(name);
        if(it == NameToId_.end())
            return nullptr;
        else
            return IdToService(it->second);
    }
    return nullptr;
}



}