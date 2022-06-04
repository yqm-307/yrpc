/**
 * @file Epoller.h
 * @author yqm-307 (979336542@qq.com)
 * @brief 使用epoll实现Poller
 * @version 0.1
 * @date 2022-06-04
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#pragma once
#include <YqmUtil/noncopyable/noncopyable.h>
#include <libyqmnet/TimerQueue.h>
#include "Poller.h"



namespace yrpc::coroutine::poller
{

typedef yrpc::coroutine::context::YRoutineFunc TaskFunc;
using namespace yrpc::coroutine;





class Epoller final:YUtil::noncopyable 
{

public:

    virtual void AddTask(yrpc::coroutine::context::YRoutineFunc&&func,void* args);
    virtual bool YieldTask();
    virtual bool Run();
    virtual void RunForever();

    void 
private:
    typedef std::vector<TaskFunc> TaskQueue;


    
    TaskQueue pending_tasks_;   //待处理任务   
    Scheduler runtime_;         //协程调度

    net::TimerQueue timer_;     //定时器队列


};

}