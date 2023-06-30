/**
 * @file Epoller.h
 * @author yqm-307 (979336542@qq.com)
 * @brief 基于 epoll 的协程调度，Epoller
 * @version 0.1
 * @date 2022-06-04
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#pragma once
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <limits>
#include <string>
#include <vector>

// #include "Poller.h"
#include "Scheduler.h"
#include "../Util/logger.h"
#include "../Util/Clock.h"
#include "../Util/comparator.h"
#include "../Util/TimeQueue.h"
#include "../Util/noncopyable.h"
#include "../Util/TcpUtil.h"
#include "../Util/Locker.h"
#include "bbt/pool_util/idpool.hpp"


namespace yrpc::coroutine::poller
{
class Epoller;
typedef int YRoutine_t;



enum EpollREventStatus  : int32_t
{
    EpollREvent_Timeout = 0,
    EpollREvent_Error = -1,
    EpollREvent_Close = -2,
};



struct RoutineSocket
{
private:
    typedef int YRoutine_t;
    typedef std::function<void(RoutineSocket*)> ConnTimeOut;
    typedef std::function<void(RoutineSocket*)> SocketTimeOut;
public:
    Epoller* scheduler; // 所属eventloop

    int             eventtype_;         //Epoll Event Type 不是事件类型，而是错误码
    struct epoll_event event_;          // epoll event 对象
    int             sockfd_;            // socket fd
    int             epollfd_;           // 所属 epoll fd
    YRoutine_t      routine_index_;     // 协程id

    /////////////////////////////
    //////  TimeOut about  //////
    /////////////////////////////
    int             connect_timeout_ms_{5000};    
    int             socket_timeout_ms_{3000};
    ConnTimeOut     conn_timeout_;
    SocketTimeOut   socket_timeout_;
    
    time_t          last_recv_time;     // 最后一次接受数据时间
    /**
     * task生命期和Socket相同，因为socket在Timer中触发超时事件，就要被断开了，此时task同时析构合情合理
     */
    yrpc::util::clock::YTimer<RoutineSocket*>::TaskSlot::Ptr timetask_;
    void** args;     /*作为拓展使用*/
};




class Epoller final : bbt::noncopyable 
{
    
    template<typename DataObj>
    using TimeTask = yrpc::util::clock::Task<DataObj>;

    template<typename Obj>
    using TimerQueue = yrpc::util::clock::YTimer<Obj>;
    
    template<typename Lock_> 
    using lock_guard = yrpc::util::lock::lock_guard<Lock_>;

    typedef yrpc::util::lock::Mutex     Mutex;
    typedef std::function<void()>       TimerTaskFunc;
    typedef yrpc::util::clock::YTimer<RoutineSocket*>::Ptr      TTaskPtr;
    typedef yrpc::util::clock::YTimer<TimerTaskFunc>::Ptr       NormalTaskPtr;
public:
    typedef yrpc::coroutine::context::YRoutineFunc TaskFunc;
    /**
     * @brief 构造一个Epoller对象
     * 
     * @param stacksize 协程栈大小
     * @param maxqueue  事件监听最大数量  
     * @param logpath   日志名
     * @param protect   是否启用内存保护
     */
    Epoller(size_t stacksize,int maxqueue,bool protect = true);
    ~Epoller();
    

    /**
     * @brief 新连接的socket，创建
     * 
     * @param sockfd 
     * @param timeout_ms 
     * @param connect_out_ms 
     * @param nodelay 
     * @return Socket* 
     */
    RoutineSocket* CreateSocket(const int sockfd,const int timeout_ms=5000,const int connect_out_ms=3000,bool noblocking = true,const bool nodelay=true);

    /**
     * @brief 释放单个socket套接字占用内存，但是socket并不是epoller的create函数创建，应有创建者负责销毁
     * 
     * @param freeptr socket指针
     */
    void DestorySocket(RoutineSocket* freeptr);


    /**
     * @brief 添加任务
     * 
     * @param func 任务函数
     * @param args 任务函数的参数
     */
    void AddTask(yrpc::coroutine::context::YRoutineFunc func,void* args = nullptr);
    void AddTask_Unsafe(yrpc::coroutine::context::YRoutineFunc func,void* args = nullptr);

    
    /**
     * @brief 挂起当前执行任务，切换到主协程，对外提供纯粹的yield接口，不保证一定切换回来
     * 
     * @return true 当前执行协程成功yield
     * @return false 当前执行协程yield失败，或者没有协程（排除调度协程，也就是主协程）正在执行
     */
    bool YieldTask();
    
    
    /**
     * @brief 将当前运行协程添加到suspend队列中，并挂起当前协程，只是挂起让出cpu等待调度器调度到本协程，保证一定切换回来
     */
    void Yield();


    /**
     * @brief 开始循环 如果没有设置forever标志位，则不阻塞，处理当前事件后直接返回；否则阻塞，一直循环
     * 
     * @return true 
     * @return false 
     */
    bool Loop();



    /**
     * @brief 使epoll循环运行，调用loop阻塞不会停止
     */
    void RunForever();


    /**
     * @brief 任务队列是否满
     * 
     * @return true 已满
     * @return false 未满
     */
    bool QueueFull();

    
    /**
     * @brief 添加一个定时任务
     * 
     * @param socket 套接字
     * @param timeout 超时时间 ms
     * @return int  0 成功; -1 失败 
     */
    int AddTimer(RoutineSocket* socket,yrpc::util::clock::Timestamp<yrpc::util::clock::ms> timeout);        //添加定时任务
    int AddTimer(RoutineSocket* socket,int timeoutms);

    /**
     * @brief 添加一个定时回调任务(thread unsafe)
     * 
     * @param func          超时任务
     * @param timeout_ms    首次超时时间(ms)
     * @param reset_time    首次触发之后重复触发时间间隔,-1 表示只触发一次
     * @param max_trigget_times     最大触发次数,-1无穷
     * @return  int         如果返回: 0 成功; -1 失败
     */
    int AddTimer(TimerTaskFunc&& func,int timeout_ms,int reset_time = -1,int max_trigget_times = 1);





    /**
     * @brief 添加一个 socket 超时定时器
     *  socket 超时比较特殊，如果每次IO都重新设置，那么触发将会非常的频繁。所以设置为定时检测是否有keep a live之类的
     *  就是下面的 ResetSocketTimer 
     * 
     * @param socket 超时定时器
     * @return int 0成功,-1失败
     */
    int AddSocketTimer(RoutineSocket* socket);


    /**
     * @brief 重置一个 Socket 超时定时器
     * 
     * @param socket 超时定时器
     * @return int 0成功,-1失败
     */
    int ResetSocketTimer(RoutineSocket* socket);

    /**
     * @brief 取消该定时任务
     * 
     * @param socket socket封装
     */
    void CancelTimer(RoutineSocket* socket);     //取消定时任务

    /**
     * @brief 取消socket超时定时器
     * 
     * @param socket 
     */
    void CancelSocketTimer(RoutineSocket* socket);
    
    /**
     * @brief 获取当前正在运行的协程句柄
     * @return YRoutine_t 主协程id
     */
    YRoutine_t GetCurrentRoutine();

    /**
     * @brief 协程数量
     * 
     * @return size_t 
     */
    size_t Size();

    /* 获取 Epoller 的id */
    int GetID();
private:

    
    /**
     * @brief 当前其他任务，注册到线程调度器中
     */
    void DoPendingList();
    
    
    /**
     * @brief 处理所有定时事件，并将socket 标志位设置为 flag，全部执行
     * 
     * @param flag 
     */
    void ResumeAll(int flag);
    
    
    /**
     * @brief 处理超时事件
     * @param next_timeout 
     */
    void DoTimeoutTask();


    /**
     * @brief 唤醒挂起的协程
     */
    void WakeUpSuspend();

private:
    typedef std::pair<TaskFunc,void*> PendingTask;
    typedef std::queue<PendingTask> PendingTaskQueue;             //新任务队列
    typedef std::queue<YRoutine_t> SuspendQueue;    //挂起协程队列

    SuspendQueue                suspend_queue_;     //协程挂起队列
    PendingTaskQueue            pending_tasks_;     //待处理任务
    // Mutex                       m_lock;             // thread safe addtask
    size_t                      max_size_;          //待处理任务队列长度
    detail::Scheduler           runtime_;           //协程调度


    // 
    TimerQueue<RoutineSocket*>      timer_;             // 协程定时事件
    TimerQueue<TimerTaskFunc>       normal_timer_;      // 超时回调任务
    Mutex                           mutex_timer_;       

    /// 单独加锁,比较频繁
    TimerQueue<RoutineSocket*>      socket_timer_;      // socket定时
    Mutex                           mutex_socket_timer_;   

    int         epollfd_;               
    bool        close_;   
    bool        forever_;     
    static bbt::pool_util::IDPool<int,true>    m_id_pool;
    int m_id;
};


class YRPCCoSchedulerHelper {
public:
    static Epoller* GetCurrentScheduler(){
        static thread_local Epoller* scheduler = nullptr;
        if (!scheduler) {
            scheduler = new Epoller(64*1024,65535);
        }
        return scheduler;
    }
};

}// namespace yrpc::coroutine::poller

#define y_scheduler yrpc::coroutine::poller::YRPCCoSchedulerHelper::GetCurrentScheduler()
#define y_scheduler_id (yrpc::coroutine::poller::YRPCCoSchedulerHelper::GetCurrentScheduler()->GetID())