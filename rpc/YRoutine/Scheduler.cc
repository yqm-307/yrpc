/**
 * @file Scheduler.cc
 * @author yqm-307 (979336542@qq.com)
 * @brief 
 * @version 0.1
 * @date 2022-06-04
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#include "Scheduler.h"
#include <assert.h>


using namespace yrpc::coroutine::detail;

YCO_Scheduler::YCO_Scheduler(size_t stacksize,bool need_memlock)
    :stack_size_(stacksize),
    need_memlock_(need_memlock),r_num_(0),
    last_routine_index_(-1),current_routine_index_(-1)
{
}

YCO_Scheduler::~YCO_Scheduler()
{}






void YCO_Scheduler::YRoutineDone()
{
    if(current_routine_index_ < 0)
        return;
    //修改node的指向，加入到空闲链表
    r_list_[current_routine_index_].Next_Node = last_routine_index_;
    r_list_[current_routine_index_].status_ = DONE;
    last_routine_index_ = current_routine_index_;
    current_routine_index_ = -1;
    r_num_--;
}


bool YCO_Scheduler::Yield()
{
    if(current_routine_index_ < 0)
        return false;   //无效的Yield
    auto Node = r_list_[current_routine_index_];
    Node.status_ = SUSPEND;
    current_routine_index_ = -1;
    Node.context_->Yield();
    return true;
}



bool YCO_Scheduler::Resume(int index)
{
    if(index >= r_list_.size())
        return false;
    auto context = r_list_[index];

    if(r_list_[index].status_==SUSPEND)
    {
        context.status_=RUNNING;
        current_routine_index_ = index;
        context.context_->Resume();
        return true;
    }
    return false;
}




YRoutine_t YCO_Scheduler::CreateRoutine(context::YRoutineFunc&& func,void* args)
{
    YRoutine_t i = -1;
    if(last_routine_index_>=0)  //只需要改变空闲槽
    {
        i = last_routine_index_;
        last_routine_index_ = r_list_[last_routine_index_].Next_Node;
        r_list_[i].context_->Make(func,args);
    }
    else//需要创建新的
    {
        i = r_list_.size();
        RoutineNode Node;
        Node.context_ = context::YRoutineContext::CreateHandle(stack_size_,func,args,
            std::bind(&YCO_Scheduler::YRoutineDone,this),need_memlock_); 
        assert(Node.context_);
        r_list_.push_back(Node);
    }

    r_list_[i].status_=SUSPEND; //挂起
    r_list_[i].Next_Node=-1;
    r_num_++;

    return i;
}


