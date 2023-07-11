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
#include "yrpc/shared/all.h"
#include "bbt/poolutil/IDPool.hpp"
#include <bbt/templateutil/BaseType.hpp>

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



struct RoutineSocket: public bbt::templateutil::BaseType<RoutineSocket>
{
private:
    typedef int YRoutine_t;
    typedef std::function<void(RoutineSocket*)> ConnTimeOut;
    typedef std::function<void(RoutineSocket*)> SocketTimeOut;
public:
    RoutineSocket();
    ~RoutineSocket(){}
    Epoller* m_scheduler; // 所属eventloop

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
    /**
     * task生命期和Socket相同，因为socket在Timer中触发超时事件，就要被断开了，此时task同时析构合情合理
     */
    yrpc::util::clock::YTimer<RoutineSocket*>::TaskSlot::Ptr timetask_;
    yrpc::detail::shared::errorcode err;
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
     * @brief 添加任务
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

    /* 将当前运行协程添加到suspend队列中，并挂起当前协程，只是挂起让出cpu等待调度器调度到本协程，保证一定切换回来*/
    void Yield();
    /* 开始循环 */
    bool Loop();
    /* 使Loop() 循环永久运行 */
    void RunForever();
    /* 任务队列是否满 */
    bool QueueFull();    
    /**
     * @brief 添加一个定时任务
     * 
     * @param socket 套接字
     * @param timeout 超时时间 ms
     * @return int  0 成功; -1 失败 
     */
    int AddTimer(RoutineSocket* socket,bbt::timer::clock::Timestamp<bbt::timer::clock::ms> timeout);        //添加定时任务
    // int AddTimer(RoutineSocket* socket,int timeoutms);

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
    /* 添加一个socket定时器 */
    int AddSocketTimer(RoutineSocket* socket);
    /* 重置一个 Socket 超时定时器 */
    int ResetSocketTimer(RoutineSocket* socket);
    /* 取消定时任务 */
    void CancelTimer(RoutineSocket* socket);
    /* 取消socket超时定时器 */
    void CancelSocketTimer(RoutineSocket* socket);
    /* 获取当前正在运行的协程句柄 */
    YRoutine_t GetCurrentRoutine();
    /* 获取当前协程数量 */
    size_t Size();
    /* 获取 epoller 的 id */
    int GetID();
    /* 获取 epoll fd */
    int GetPollFd();
private:
    /* 当前其他任务，注册到线程调度器中 */
    void DoPendingList();
    /* 处理所有定时事件，并将socket 标志位设置为 flag，全部执行 */
    void ResumeAll(int flag);
    /* 处理超时事件 */
    void DoTimeoutTask();
    /* 唤醒挂起的协程 */
    void WakeUpSuspend();
private:
    typedef std::pair<TaskFunc,void*> PendingTask;
    typedef std::queue<PendingTask> PendingTaskQueue;             //新任务队列
    typedef std::queue<YRoutine_t> SuspendQueue;    //挂起协程队列

    SuspendQueue                m_suspend_queue;     //协程挂起队列
    PendingTaskQueue            m_pending_tasks;     //待处理任务
    Mutex                       m_lock;             // thread safe addtask
    size_t                      m_max_size;          //待处理任务队列长度
    detail::Scheduler           m_runtime;           //协程调度


    // 
    TimerQueue<RoutineSocket*>      m_routine_timer;             // 协程定时事件
    TimerQueue<TimerTaskFunc>       m_comm_timer;      // 超时回调任务
    Mutex                           m_mutex_timer;       

    /// 单独加锁,比较频繁
    TimerQueue<RoutineSocket*>      m_socket_timer;      // socket定时
    Mutex                           m_mutex_socket_timer;   

    int         m_epollfd;               
    bool        m_closed;   
    bool        m_forever_flag;     
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