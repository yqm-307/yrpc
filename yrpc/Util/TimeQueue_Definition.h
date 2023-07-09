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
#include "TimeQueue.h"
#include "yrpc/YRoutine/Epoller.h"
#include <assert.h>
#include <thread>
#include <memory>

namespace yrpc::util::clock
{

template<class TaskObject>
int YTimer<TaskObject>::ThreadSleepFor(clock::ms sleep_ms)
{
    if(sleep_ms.count()<0)
        return -1;
    else if(sleep_ms.count()==0)
        return 0;
    std::this_thread::sleep_for(sleep_ms);
    return sleep_ms.count();
}

template<class TaskObject>
int YTimer<TaskObject>::ThreadSleepUntil(Timestamp<ms> timepoint)
{
    if(clock::expired<ms>(std::move(timepoint)))
        return -1;
    else{
        std::this_thread::sleep_until(timepoint);
    }
    return 0;
}

template<class TaskObject>
typename YTimer<TaskObject>::Ptr YTimer<TaskObject>::AddTask(Timestamp<ms> expired,TaskObject data)
{
    //if(clock::expired(expired)) //超时任务，创建失败
    //    return NULL;
    Ptr slot =TaskSlot::CreateTaskSlotWithSharedOfThis(expired,data);
    min_heap_.push(slot);
    return slot;
}

template<class TaskObject>
bool YTimer<TaskObject>::AddTask(Ptr task)
{
    if (task == nullptr)
        return false;
    min_heap_.push(task);
    return true;
}

template<class TaskObject>
void YTimer<TaskObject>::CancelTask(Ptr task)
{
    task->Cancel();
}

template<class TaskObject>
void YTimer<TaskObject>::GetAllTimeoutTask(std::vector<Ptr>& sockets)
{
    Ptr p=nullptr;

    if(min_heap_.empty())
        return;
    while(!min_heap_.empty() && clock::expired<ms>(min_heap_.top()->GetValue()))
    {
        auto p = PopTimeTask();
        if( !(p->Is_Canceled()) )  //不是被取消的
        {
            sockets.push_back(p);
        }
        // DEBUG("[YRPC][YTimer<TaskObject>::GetAllTimeoutTask] task is be cancel!");
    }
}

template<class TaskObject>
typename YTimer<TaskObject>::Ptr YTimer<TaskObject>::PopTimeTask()
{
    // 删除定时任务时检测是否为重复触发的
    if(min_heap_.empty())
        return nullptr;
    auto p = min_heap_.top();
    min_heap_.pop();

    if (p->Reset()) // 重复触发      
        min_heap_.push(p);
    return p;
}



template<class TaskObject>
typename YTimer<TaskObject>::Ptr YTimer<TaskObject>::GetATimeoutTask()
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


template<class TaskObject>
void YTimer<TaskObject>::GetAllTask(std::vector<Ptr>& sockets)
{
    Ptr p=nullptr;
    while(!min_heap_.empty())
        if( !(p = PopTimeTask())->Is_Canceled() )  //不是被取消的
            sockets.push_back(p);
}




////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////###    TimeTask    ###///////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////
template<typename DataObject>
Task<DataObject>::Task(clock::Timestamp<ms>& timepoint,DataObject data,int trigger,int max_times)
    :m_is_canceled(false),
    m_trigger_interval(trigger),
    m_max_trigger_times(max_times),
    m_data(data)
{
    this->SetValue(timepoint);
}



template<typename DataObject>
typename Task<DataObject>::Ptr Task<DataObject>::CreateTaskSlotWithSharedOfThis(clock::Timestamp<ms>&timepoint,DataObject data,int trigger,int max_times)
{
    return std::make_shared<Task>(timepoint,data,trigger,max_times);
}


template<typename DataObject>
bool Task<DataObject>::operator>(const comparator<clock::Timestamp<ms>> &rvalue) const
{
    return m_it > rvalue.GetValue();
}



template<typename DataObject>
bool Task<DataObject>::operator==(const comparator<clock::Timestamp<ms>> &rvalue) const
{
    return m_it == rvalue.GetValue();
}



template<typename DataObject>
void Task<DataObject>::Cancel() { m_is_canceled = true; }



template<typename DataObject>
bool Task<DataObject>::Is_Canceled() { return m_is_canceled; }



template<typename DataObject>
DataObject& Task<DataObject>::Data() { return m_data; }



template<typename DataObject>
void Task<DataObject>::SetAutoReset(int tick_ms)
{
    m_trigger_interval = tick_ms;
}


template<typename DataObject>
int Task<DataObject>::GetReset()
{
    return m_trigger_interval;
}


template<typename DataObject>
bool Task<DataObject>::Reset()
{
    if ( m_trigger_interval < 0)
        return false;

    m_it = m_it + yrpc::util::clock::ms(m_trigger_interval);

    if (m_max_trigger_times<0)
        return true;
    else if( m_max_trigger_times == 0 )
        return false;
    else
        m_max_trigger_times--;
    
    return true;
}


template<typename DataObject>
Task<DataObject>::Task(const Task& r)
{
    this->m_data = r->m_data;
    this->m_is_canceled = r->m_is_canceled;
    this->m_it = r->m_it;
    this->m_trigger_interval = r->m_trigger_interval;
}

template<typename DataObject>
Task<DataObject>::Task(const Task&& r)
{
    this->m_data = r->m_data;
    this->m_is_canceled = r->m_is_canceled;
    this->m_it = r->m_it;
    this->m_trigger_interval = r->m_trigger_interval;
}



}