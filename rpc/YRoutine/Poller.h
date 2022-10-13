// /**
//  * @file Poller.h
//  * @author yqm-307 (979336542@qq.com)
//  * @brief Poller的接口类
//  * @version 0.1
//  * @date 2022-06-04
//  * 
//  * @copyright Copyright (c) 2022
//  * 
//  */
// #pragma once
// #include "Scheduler.h"
// #include "../Util/all.h"


// namespace yrpc::coroutine::poller
// {




// class Epoller final:util::noncopyable::noncopyable 
// {

// public:
//     /**
//      * @brief 构造一个Epoller对象
//      * 
//      * @param stacksize 协程栈大小
//      * @param maxqueue  事件监听最大数量  
//      * @param logpath   日志名
//      * @param protect   是否启用内存保护
//      */
//     Epoller(size_t stacksize,int maxqueue,std::string logpath = "server.log",bool protect = true);
//     ~Epoller();
    

//     /**
//      * @brief 新连接的socket，创建
//      * 
//      * @param sockfd 
//      * @param timeout_ms 
//      * @param connect_out_ms 
//      * @param nodelay 
//      * @return Socket* 
//      */
//     RoutineSocket* CreateSocket(const int sockfd,const int timeout_ms=5000,const int connect_out_ms=3000,bool noblocking = true,const bool nodelay=true);

//     /**
//      * @brief 释放单个socket套接字占用内存，但是socket并不是epoller的create函数创建，应有创建者负责销毁
//      * 
//      * @param freeptr socket指针
//      */
//     void DestorySocket(RoutineSocket* freeptr);


//     /**
//      * @brief 添加任务
//      * 
//      * @param func 任务函数
//      * @param args 任务函数的参数
//      */
//     void AddTask(yrpc::coroutine::context::YRoutineFunc&&func,void* args);
//     void AddTask(yrpc::coroutine::context::YRoutineFunc&func,void* args);
    
//     /**
//      * @brief 挂起当前执行任务，切换到主协程，对外提供纯粹的yield接口，不保证一定切换回来
//      * 
//      * @return true 当前执行协程成功yield
//      * @return false 当前执行协程yield失败，或者没有协程（排除调度协程，也就是主协程）正在执行
//      */
//     bool YieldTask();
//     /**
//      * @brief 将当前运行协程添加到suspend队列中，并挂起当前协程，只是挂起让出cpu等待调度器调度到本协程，保证一定切换回来
//      */
//     void Yield();
//     /**
//      * @brief 开始循环 如果没有设置forever标志位，则不阻塞，处理当前事件后直接返回；否则阻塞，一直循环
//      * 
//      * @return true 
//      * @return false 
//      */
//     bool Loop();



//     /**
//      * @brief 使epoll循环运行，调用loop阻塞不会停止
//      */
//     void RunForever();


//     /**
//      * @brief 任务队列是否满
//      * 
//      * @return true 已满
//      * @return false 未满
//      */
//     bool QueueFull();

    
//     /**
//      * @brief 添加一个定时任务
//      * 
//      * @param socket 套接字
//      * @param timeout 超时时间 ms
//      * @return int 0 成功; -1 失败 
//      */
//     int AddTimer(RoutineSocket* socket,yrpc::util::clock::Timestamp<yrpc::util::clock::ms> timeout);        //添加定时任务


//     /**
//      * @brief 取消该定时任务
//      * 
//      * @param socket socket封装
//      */
//     void CancelTimer(RoutineSocket* socket);     //取消定时任务
    
//     /**
//      * @brief 获取当前正在运行的协程句柄
//      * @return YRoutine_t 主协程id
//      */
//     YRoutine_t GetCurrentRoutine();



// private:
//     typedef yrpc::util::clock::YTimer<RoutineSocket*>::Ptr TTaskPtr;
//     /**
//      * @brief 当前其他任务，注册到线程调度器中
//      */
//     void DoPendingList();
//     /**
//      * @brief 处理所有定时事件，并将socket 标志位设置为 flag，全部执行
//      * 
//      * @param flag 
//      */
//     void ResumeAll(int flag);
//     /**
//      * @brief 处理超时事件
//      * @param next_timeout 
//      */
//     void DoTimeoutTask();
//     /**
//      * @brief 唤醒挂起的协程
//      */
//     void WakeUpSuspend();

// private:
//     typedef std::pair<TaskFunc,void*> Task;
//     typedef std::queue<Task> TaskQueue;             //新任务队列
//     typedef std::queue<YRoutine_t> SuspendQueue;    //挂起协程队列

//     SuspendQueue suspend_queue_;//协程挂起队列
//     TaskQueue pending_tasks_;   //待处理任务
//     int max_size_;              //待处理任务队列长度
//     detail::Scheduler m_scheduler;         //协程调度


//     int     m_epollfd;               
//     bool    m_closed;   
//     bool    m_runforever;     
// };

// }