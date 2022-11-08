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
        if( !(p = PopTimeTask())->Is_Canceled())  //不是被取消的
            sockets.push_back(p);
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
    :data_(data),
    is_canceled_(false),
    trigger_interval_(trigger),
    max_trigger_times_(max_times)
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
    return it_ > rvalue.GetValue();
}



template<typename DataObject>
bool Task<DataObject>::operator==(const comparator<clock::Timestamp<ms>> &rvalue) const
{
    return it_ == rvalue.GetValue();
}



template<typename DataObject>
void Task<DataObject>::Cancel() { is_canceled_ = true; }



template<typename DataObject>
bool Task<DataObject>::Is_Canceled() { return is_canceled_; }



template<typename DataObject>
DataObject& Task<DataObject>::Data() { return data_; }



template<typename DataObject>
void Task<DataObject>::SetAutoReset(int tick_ms)
{
    trigger_interval_ = tick_ms;
}


template<typename DataObject>
int Task<DataObject>::GetReset()
{
    return trigger_interval_;
}


template<typename DataObject>
bool Task<DataObject>::Reset()
{
    if ( trigger_interval_ < 0)
        return false;

    it_ = it_ + yrpc::util::clock::ms(trigger_interval_);

    if (max_trigger_times_<0)
        return true;
    else if( max_trigger_times_ == 0 )
        return false;
    else
        max_trigger_times_--;
    
    return true;
}


template<typename DataObject>
Task<DataObject>::Task(const Task& r)
{
    this->data_ = r->data_;
    this->is_canceled_ = r->is_canceled_;
    this->it_ = r->it_;
    this->trigger_interval_ = r->trigger_interval_;
}

template<typename DataObject>
Task<DataObject>::Task(const Task&& r)
{
    this->data_ = r->data_;
    this->is_canceled_ = r->is_canceled_;
    this->it_ = r->it_;
    this->trigger_interval_ = r->trigger_interval_;
}



}