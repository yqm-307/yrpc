#pragma once
#include "./Clock.h"
#include <string.h>

namespace yrpc::util::id
{

#define Second_NS (1000000000) 
#define Minute_NS (Second_NS*60)
#define Hours_NS  (Minute_NS*60)

#define Second_US (1000000)
#define Minute_US (Second_US*60)
#define Hours_US  (Minute_US*60)


#define Second_MS (1000)
#define Minute_MS (Second_MS*60)
#define Hours_MS  (Minute_MS*60)


class GenerateID
{
public:

/**
 * @brief 永久失效性uid
 * 
 * @return uint64_t uid
 */
static uint64_t GetIDuint64()
{

    auto p = yrpc::util::clock::now<yrpc::util::clock::ns>();
    static uint64_t per = p.time_since_epoch().count();
    static int index=0;
    uint64_t id = p.time_since_epoch().count();
    if(per == id)
        id = id/*24小时有效*/+index++;
    else
    {//下一个时间段
        per = id;
        index=0;
    }
    return id;
}

/**
 * @brief 提供时效为5分钟的uid，10分钟不保证唯一性
 * 
 * @return uint32_t 唯一性时效为10分钟uid，失败返回0
 */
static uint32_t GetIDuint32()
{
    auto p = yrpc::util::clock::now<yrpc::util::clock::ns>();
    static uint32_t per = (p.time_since_epoch().count())/1000%Hours_MS;  //当前小时
    static int index=0;
    uint32_t id = (p.time_since_epoch().count())/1000%Hours_MS;
    if(per == id)
        id = index >= Hours_MS ? 0 : id+10*Minute_MS/*10分钟*/+index++;
    else
    {//下一个时间段
        per = id;
        index=0;
    }
    return id;
}



/**
 * @brief 返回1周内的uid，同一毫秒时间戳下可以重复千万次
 * 
 * @return uint32_t 大于0的时间戳，错误为0
 */
static uint32_t GetIDuint32_unsafe()
{   
    auto p = yrpc::util::clock::now<yrpc::util::clock::ns>();
    static uint32_t per = (p.time_since_epoch().count())/1000/1000%(7*24*60*60*1000);  // 24小时内的ms时间
    static int index = 0;
    uint32_t now_ms = (p.time_since_epoch().count())/1000%(1000*1000*1000);
    if(per == now_ms)
        now_ms = index >= 7*24*60*60*1000 ? 0 : now_ms+(7*24*60*60*1000)+index++;
    else
    {//下一个时间段
        per = now_ms;
        index=0;
    }
    return now_ms;
}




};

}