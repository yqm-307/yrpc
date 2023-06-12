// /**
//  * @file logger.h
//  * @author yqm-307 (979336542@qq.com)
//  * @brief 日志实现直接使用libyqmnet
//  * @version 0.1
//  * @date 2022-06-04
//  * 
//  * @copyright Copyright (c) 2022
//  */
// #pragma once
// #include <queue>
// #include <thread>
// #include <atomic>
// #include <memory>
// #include <condition_variable>   //mutex
// #include "./noncopyable.h"
// #ifndef LOG_LEVEL
// #define LOG_LEVEL -1
// #endif

// #define ARRAY_NUM 8
// #define ARRAY_SIZE 1024*4   //4kb   linux下每次读写为4kb时，用户cpu时间和系统cpu时间最短
// #define LOG_STDOUT

// /* 
//     这个宏是否定义很关键，它决定了日志的实现方式，当前日志有两种实现方式，皆为异步日志。

//     一、完全异步，开一个线程，线程会while(true) 循环 flush 阻塞队列里面的log数据，如果
//     日志量小的情况，会导致严重的cpu空转，就是cpu占用率飙升，但是实时性强。 

//     二、半异步半同步，线程写入日志时，调用log就会检查当前双循环队列缓冲区，并且由后台线
//     程定时或者溢出时触发flush，这种利用缓存，效率上较为出色，有效减少了系统调用次数，但
//     是，可能会导致内存占用增加（原因是，目前双循环队列会增大，但是不会动态调整缩小），由
//     于设置了超时(毫秒级)，也不会导致延迟特别高，但是对于日志量非常大的情况不如方案(一)。
//  */
// // #define YNET_LOG_BUFFER


// namespace yrpc::util::logger
// {

// enum LOGLEVEL : int32_t
// {
//     LOG_TRACE=0,        //跟踪点
//     LOG_DEBUG,          //调试
//     LOG_INFO,           //消息
//     LOG_WARN,           //警告
//     LOG_ERROR,          //错误
//     LOG_FATAL,          //致命错误
// };



// //缓冲日志

// class Logger : yrpc::util::noncopyable::noncopyable
// {
// public:
// #ifdef YRPC_LOG_NAME
//     static Logger* GetInstance(std::string name = YRPC_LOG_NAME);
// #else
//     static Logger* GetInstance(std::string name = "./log.txt");
// #endif
//     void Log(LOGLEVEL level,const std::string log);
//     static void SetFileName(std::string name);

// private:
//     Logger(std::string);
//     ~Logger();

// #ifdef YNET_LOG_BUFFER
//     /**
//      * @brief 获取一个已满buffer 的指针
//      * 
//      * @return const char* 返回一个已满的buffer的首地址
//      */
//     const char* GetFullArray();

//     /**
//      * @brief 返回当前正在写入的buffer的首指针
//      * 
//      * @return char* buffer指针
//      */
//     char* workarray(){return _buffers[_nowindex].second;}
    
//     /**
//      * @brief 将当前index想起推进，如果当前队列已满，创建新节点，否则直接向后移动一位
//      */
//     void next();
    
//     /**
//      * @brief Pendingwriteindex 前进
//      */
//     void nextPending();
//     /**
//      * @brief 是否有一个buffer是满的
//      * 
//      * @return true 
//      * @return false 
//      */
//     bool hasfulled(){return _pendingwriteindex!=_nowindex;}
    
//     /**
//      * @brief 将下标为index的buffer中的数据写入到磁盘
//      * 
//      * @param index 
//      * @return int 返回0成功
//      */
//     int flushbuffer(int index);

//     /**
//      * @brief 将下标为index的buffer置空
//      * 
//      */
//     int resetbuffer(int index);
// #else
//     bool Dequeue(std::string& str);
// #endif
//     void Enqueue(std::string log);
   
    

//     //todo flush 服务器关闭前，主动冲洗剩余内存
// private:

// #ifdef YNET_LOG_BUFFER
//     //buffer，第一个值是下一个节点下标。第二个值是储存数据
//     std::vector<std::pair<int,char*>> _buffers;    //缓冲区
//     int _nowsize;
//     int _pendingwriteindex;     //待写入
//     int _nowindex;              //当前
//     std::condition_variable _cond;
//     std::mutex _condlock;
// #endif
//     std::queue<std::string> _queue;
//     std::thread* _writeThread;      //不断dequeue
//     std::mutex _mutex;
//     std::condition_variable _wakeup;
//     std::string filename;           //文件名可配置
//     int _openfd;                    //文件


// };

// std::string format(const char* fmt, ...);


// #define TRACE(fmt, ...)     yrpc::util::logger::Logger::GetInstance()->Log(yrpc::util::logger::LOG_TRACE, yrpc::util::logger::format(yrpc::util::logger::format("[%s:%d] %s",__func__,__LINE__,fmt).c_str(),##__VA_ARGS__))
// #define DEBUG(fmt, ...)     yrpc::util::logger::Logger::GetInstance()->Log(yrpc::util::logger::LOG_DEBUG, yrpc::util::logger::format(yrpc::util::logger::format("[%s:%d] %s",__func__,__LINE__,fmt).c_str(),##__VA_ARGS__))
// #define INFO(fmt, ...)      yrpc::util::logger::Logger::GetInstance()->Log(yrpc::util::logger::LOG_INFO,  yrpc::util::logger::format(yrpc::util::logger::format("[%s:%d] %s",__func__,__LINE__,fmt).c_str(),##__VA_ARGS__))
// #define WARN(fmt, ...)      yrpc::util::logger::Logger::GetInstance()->Log(yrpc::util::logger::LOG_WARN,  yrpc::util::logger::format(yrpc::util::logger::format("[%s:%d] %s",__func__,__LINE__,fmt).c_str(),##__VA_ARGS__))
// #define ERROR(fmt, ...)     yrpc::util::logger::Logger::GetInstance()->Log(yrpc::util::logger::LOG_ERROR, yrpc::util::logger::format(yrpc::util::logger::format("[%s:%d] %s",__func__,__LINE__,fmt).c_str(),##__VA_ARGS__))
// #define FATAL(fmt, ...)     yrpc::util::logger::Logger::GetInstance()->Log(yrpc::util::logger::LOG_FATAL, yrpc::util::logger::format(yrpc::util::logger::format("[%s:%d] %s",__func__,__LINE__,fmt).c_str(),##__VA_ARGS__))



// }
#pragma once
#include <bbt/Logger/Logger.hpp>