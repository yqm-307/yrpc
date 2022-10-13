/**
 * @file Clock.h
 * @author yqm-307 (979336542@qq.com)
 * @brief 用std::chrono 来封装时间工具。
 * @version 0.1
 * @date 2022-06-04
 * 
 * @copyright Copyright (c) 2022
 * 
 */


#pragma once
#include <iostream>
#include <chrono>
namespace yrpc::util::clock
{
//函数占位符

/**
 *  时间单位
 */
using namespace std::chrono_literals;
typedef std::chrono::system_clock system_clock;

typedef std::chrono::nanoseconds ns;
typedef std::chrono::milliseconds ms;
typedef std::chrono::microseconds us;
typedef std::chrono::seconds s;
typedef std::chrono::minutes min;
typedef std::chrono::hours h;


/**
 * 时间戳
 */
template<class T>
using Timestamp = std::chrono::time_point<std::chrono::system_clock,T>;


/**
 * @brief 获取当前时间戳
 * @return Timestamp 时间戳
 */
template<class timeaccuracy>
inline Timestamp<timeaccuracy> utcnow()
{
    return std::chrono::time_point_cast<timeaccuracy>(std::chrono::system_clock::now());
}


template<class timeaccuracy>
inline Timestamp<timeaccuracy> now()
{
    return std::chrono::time_point_cast<timeaccuracy>(std::chrono::system_clock::now() + h(8));
}



/**
 * @brief 当前时间加interval后的时间戳
 * 
 * @param interval 加上多久时间（单位ns）
 * @return Timestamp 添加之后的时间戳
 */
template<class timeaccuracy,class Tsp = Timestamp<timeaccuracy>>
inline Tsp nowAfter(timeaccuracy interval)
{ return now<timeaccuracy>() + interval; }





/**
 * @brief 当前时间减去interval后的时间戳
 * 
 * @param interval 减去的多久时间（单位ns）
 * @return Timestamp 减去之后的时间戳
 */
template<class timeaccuracy,class Timestamp>
inline Timestamp nowBefore(timeaccuracy interval)
{ return now<timeaccuracy>() - interval; }



/**
 * @brief 从1970年1月1日0时，到ts的ms数
 * 
 * @param ts 某个时间点
 * @return time_t 从UTC 到 ts 的毫秒数
 */
inline time_t utcms(Timestamp<ns>&& ts)
{
    return ts.time_since_epoch().count()/1000/1000;
}


/**
 * @brief 获取ts所在月的日期
 * 
 * @param ts 某时间点
 * @return time_t 日期 （0-30）
 */
inline time_t day(Timestamp<ns>&& ts= now<ns>())
{
    time_t tt = system_clock::to_time_t(ts);
    tm utc_tm = *gmtime(&tt);
    return utc_tm.tm_mday;
}


/**
 * @brief 获取ts所在年份的月份
 * 
 * @param ts 某时间点
 * @return time_t 月份 （0-11）
 */
inline time_t month(Timestamp<ns>&& ts=now<ns>())
{
    time_t s = utcms(std::move(ts))/1000;    //ms
    return (s%(31556952)/(2629746));
}


/**
 * @brief 获取ts所在日期的几点钟
 * 
 * @param ts 某时间点
 * @return time_t (0-23)
 */
inline time_t hour(Timestamp<ns>&& ts=now<ns>())
{
    time_t s = utcms(std::move(ts))/1000;    //ms
    return (s%(86400))/(3600);
}

/**
 * @brief 获取ts的年份
 * 
 * @param ts 某时间点
 * @return time_t 当前年份
 */
inline time_t year(Timestamp<ns>&& ts=now<ns>())
{
    time_t tt = system_clock::to_time_t(ts);
    tm utc_tm = *gmtime(&tt);
    return 1900+utc_tm.tm_year;

}

/**
 * @brief 获取所在的分钟
 * 
 * @param ts 
 * @return time_t 
 */
inline time_t minute(Timestamp<ns>&& ts=now<ns>())
{
    time_t tt = system_clock::to_time_t(ts);
    tm utc_tm = *gmtime(&tt);
    return utc_tm.tm_min;
}

/**
 * @brief 获取所在的秒
 * 
 * @param ts 
 * @return time_t 
 */
inline time_t second(Timestamp<ns>&& ts=now<ns>())
{
    time_t tt = system_clock::to_time_t(ts);
    tm utc_tm = *gmtime(&tt);
    return utc_tm.tm_sec;
}

/**
 * @brief 获取所在的毫秒
 * 
 * @param ts 
 * @return time_t 
 */
inline time_t millisecond(Timestamp<ns>&& ts=now<ns>())
{
    time_t ms = utcms(std::move(ts));
    return (ms%1000);
}

inline std::string getnow_str()
{
    return std::to_string(year())+'/'+std::to_string(month()+1)+'/'+std::to_string(day())+'/'
            +std::to_string(hour())+'/'+std::to_string(minute())+'/'+std::to_string(second())+'/'+std::to_string(millisecond());
}


/**
 * @brief ts 是否小于 now
 * 
 * @param ts 
 * @return true ts超时了,false ts未超时
 */
template<class type,class Tsp = Timestamp<type>>
inline bool expired(Tsp ts)
{
    if( now<type>() >= ts )
        return true;
    else
        return false;
}

}

