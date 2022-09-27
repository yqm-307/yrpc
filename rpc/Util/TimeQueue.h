/**
 * @file TimeQueue.h
 * @author yqm-307 (979336542@qq.com)
 * @brief TimeQueue和YTimer不是一个概念，timerqueue是连带定时间任务队列、定时器、异步回调事件池、线程池的完整实现，可以支持异步回调
 *  但是，其实性能有损耗，而且不够定制化，在协程环境可能效果不佳。所以在epoll的协程封装，就要定制化，写一个单一的定时事件队列（heap实现）
 * 
 * 讲解一下，这个task对象的生命期管理，因为每个socket对象都对应一个超时节点（心跳、连接超时之类的），所以socket存在期间都应该对应一个计
 * 时器，所以生命期也应该由socket管理。仔细看就会发现，一个 timetask 对象只会被两个地方持有，一个就是YTimer，另一个就是socket了。而YTimer
 * 处理超时事件之前，socket正常情况肯定不会关闭，就算关闭，也会取消超时任务（取消后两个持有者都会释放所有权，被系统回收资源）。
 * 
 * @version 0.1
 * @date 2022-06-04
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#pragma once
// #include <libyqmnet/TimerQueue.h>
#include "../Util/comparator.h"
#include "Clock.h"
#include <queue>
#include <vector>
#include <memory>
#include <algorithm>

namespace yrpc::util::clock
{



typedef uintptr_t ttask_t;
// typedef net::TimerQueue TimerQueue;
typedef struct Socket;


template<class Item>
class Task:public comparator<clock::Timestamp<ms>>
{
public:
    typedef std::shared_ptr<Task> Ptr;
    Task(clock::Timestamp<ms>& timepoint,Item data)
        :data_(data),
        is_canceled_(false)
    {
        this->SetValue(timepoint);
    }
    ~Task(){}
    static Ptr CreateTaskSlotWithSharedOfThis(clock::Timestamp<ms>&tp,Item data_)
    {
        return std::make_shared<Task>(tp,data_); 
    }
    bool operator>(const comparator<clock::Timestamp<ms>>& rvalue) const
    {
        return it_>rvalue.GetValue();
    }
    bool operator==(const comparator<clock::Timestamp<ms>>& rvalue) const
    {
        return it_==rvalue.GetValue();
    }
    void cancel(){is_canceled_ = true;}
    bool is_canceled(){return is_canceled_;}
    Item& data(){return data_;}
protected:
    bool is_canceled_;
    Item data_;
    //Timestamp timeout;    基类元素
};



/**
 * @brief 泛用的定时器队列
 * 
 * @tparam TaskData 定时器保存的值，该超时队列自动包装 {id : {time,TaskData}}  
 */
template<class TaskData>
class YTimer
{
public:
    /**
     * @brief 定时任务
     */

    typedef Task<TaskData> TaskSlot;
    typedef std::shared_ptr<TaskSlot> Ptr;
    YTimer():min_heap_([](Ptr lval,Ptr rval){return (*(lval))>(*(rval));}){}
    ~YTimer(){}


    /**
     * @brief 添加定时器事件
     * 
     * @param expired 超时时间点
     * @param socket_t socket对象
     * @return Ptr taskslot 的智能指针
     */
    Ptr AddTask(clock::Timestamp<ms> expired,TaskData socket_t);
    
    /**
     * @brief 取消节点，但是没有删除，只是改变标志位
     * 
     * @param timetask taskslot的智能指针
     */
    void CancelTask(Ptr timetask);

    
    //void ResetTask(clock);


    /**
     * @brief 调用此函数休眠sleep_ms
     * @param sleep_ms 休眠时间 sleep 毫秒
     */
    int ThreadSleepFor(clock::ms sleep_ms);


    /**
     * @brief 休眠到指定时间
     * 
     * @param timepoint 指定时间点
     * @return int -1表示已经超时了，0表示成功
     */
    int ThreadSleepUntil(clock::Timestamp<ms> timepoint);

    
    /**
     * @brief 获取所有超时任务
     * 
     * @param sockets 入参，保存所有超时的socket 在sockets中
     */
    void GetAllTimeoutTask(std::vector<TaskData>& sockets);


    /**
     * @brief 获取所有未取消的定时器事件
     * 
     * @param sockets 返回值是vector
     */
    void GetAllTask(std::vector<TaskData>& sockets);

    /**
     * @brief 获取处于队首的一个超时任务
     * 
     * @return Ptr 处于队首的超时任务的智能指针，如果队列为空，或者队首任务没有超时，则返回nullptr指针
     */
    Ptr GetATimeoutTask();


    
private:
    /**
     * @brief 弹出一个节点
     * 
     * @return decltype(p) 
     */
    Ptr PopTimeTask();

    std::priority_queue<Ptr,std::vector<Ptr>,std::function<bool(Ptr,Ptr)>> min_heap_;    //最小堆
};



}



#include "TimeQueue_Definition.h"
