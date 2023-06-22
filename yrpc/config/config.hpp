#pragma once
#include <bbt/config/GlobalConfig.hpp>
#include <bbt/Define.hpp>


namespace yrpc::config
{

enum SysCfgType
{
    THREAD_NUM = 1,
};

#define Entry(flag) { flag, #flag }

static std::unordered_map<SysCfgType,std::string>  SysCfg = {
    Entry(THREAD_NUM),
}; 

#undef Entry
}


#define YRPC_CONFIG_QUICK_SET_ENTRY(type, value, entry) (BBT_CONFIG()->GetDynamicCfg()->SetEntry<type>( \
        yrpc::config::SysCfg[entry], value))

