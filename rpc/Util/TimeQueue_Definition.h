/**
 * @file TimeQueue.cc
 * @author yqm-307 (979336542@qq.com)
 * @brief 
 * @version 0.1
 * @date 2022-06-05
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#pragma once
#include <thread>
#include "TimeQueue.h"
#include <memory>

namespace yrpc::util::clock
{

template<class Socket>
int YTimer<Socket>::ThreadSleepFor(clock::ms sleep_ms)
{
    if(sleep_ms.count()<0)
        return -1;
    else if(sleep_ms.count()==0)
        return 0;
    std::this_thread::sleep_for(sleep_ms);
    return sleep_ms.count();
}

template<class Socket>
int YTimer<Socket>::ThreadSleepUntil(Timestamp<ms> timepoint)
{
    if(clock::expired<ms>(std::move(timepoint)))
        return -1;
    else{
        std::this_thread::sleep_until(timepoint);
    }
    return 0;
}

template<class Socket>
typename YTimer<Socket>::Ptr YTimer<Socket>::AddTask(Timestamp<ms> expired,Socket socket_t)
{
    //if(clock::expired(expired)) //超时任务，创建失败
    //    return NULL;
    Ptr slot =TaskSlot::CreateTaskSlotWithSharedOfThis(expired,socket_t);
    min_heap_.push(slot);
    return slot;
}

template<class Socket>
void YTimer<Socket>::CancelTask(Ptr task)
{
    task->cancel();
}

template<class Socket>
void YTimer<Socket>::GetAllTimeoutTask(std::vector<Socket>& sockets)
{
    Ptr p=nullptr;

    if(min_heap_.empty())
        return;
    while(!min_heap_.empty() && clock::expired<ms>(min_heap_.top()->GetValue()))
        if( !(p = PopTimeTask())->is_canceled())  //不是被取消的
            sockets.push_back(p->data());

}

template<class Socket>
typename YTimer<Socket>::Ptr YTimer<Socket>::PopTimeTask()
{
    if(min_heap_.empty())
        return nullptr;
    Ptr p = min_heap_.top();
    min_heap_.pop();
    return p;
}



template<class Socket>
typename YTimer<Socket>::Ptr YTimer<Socket>::GetATimeoutTask()
{
    if(min_heap_.size() == 0)
        return nullptr;
    auto top = min_heap_.top();
    
    if(clock::expired<ms,Timestamp<ms>>(top->GetValue()))
    {
        if(top->is_canceled()){
            PopTimeTask();
        }
        else
            return PopTimeTask();
    }
    return nullptr;
}


template<class Socket>
void YTimer<Socket>::GetAllTask(std::vector<Socket>& sockets)
{
    Ptr p=nullptr;
    while(!min_heap_.empty())
        if( !(p = PopTimeTask())->is_canceled() )  //不是被取消的
            sockets.push_back(p->data());
}






}