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