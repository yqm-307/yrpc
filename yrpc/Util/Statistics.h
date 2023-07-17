/*
    统计宏，debug专用
*/


#pragma once
#include <iostream>

#define YRPC_DEBUG

#define StatisticsValue( type , name )  \
public:\
    type Get##name()\
    {\
        return m_##name;\
    }\
    void Set##name(type val)\
    {\
        m_##name = val;\
    }\
    type Add##name(type val)\
    {\
        return (m_##name += val);\
    }\
private:\
    type m_##name\





#ifdef YRPC_DEBUG

namespace yrpc::util::statistics
{
class ByteRecord;

}


// 用来做流量监视的，方便后续调试
class yrpc::util::statistics::ByteRecord{
public: 
    ByteRecord():m_recv_bytes(0),m_send_bytes(0){}

    StatisticsValue( uint64_t , recv_bytes );
    StatisticsValue( uint64_t , send_bytes );
};

#endif


// 防止源代码污染
#undef StatisticsValue