/**
 * @file Poller.h
 * @author yqm-307 (979336542@qq.com)
 * @brief Poller的接口类
 * @version 0.1
 * @date 2022-06-04
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#pragma once
#include "Scheduler.h"


namespace yrpc::coroutine::poller
{

class Poller
{
public:
    Poller() {}
    virtual ~Poller() {}

    virtual void AddTask(yrpc::coroutine::context::YRoutineFunc&&func,void* args) = 0;
    virtual bool YieldTask()=0;
    virtual bool Loop()=0;
    virtual void RunForever()=0;

};

}