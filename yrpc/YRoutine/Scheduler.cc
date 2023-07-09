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
    :m_max_stack_size(stacksize),
    m_need_memlock_flag(need_memlock),m_routinue_num(0),
    m_last_routine_index(-1),m_current_routine_index(-1)
{
}

YCO_Scheduler::~YCO_Scheduler()
{}






void YCO_Scheduler::YRoutineDone()
{
    if(m_current_routine_index < 0)
        return;
    //修改node的指向，加入到空闲链表
    m_routinue_list[m_current_routine_index].Next_Node = m_last_routine_index;
    m_routinue_list[m_current_routine_index].status_ = DONE;
    m_last_routine_index = m_current_routine_index;
    m_current_routine_index = -1;
    m_routinue_num--;
}


bool YCO_Scheduler::Yield()
{
    if(m_current_routine_index < 0)
        return false;   //无效的Yield
    auto Node = m_routinue_list[m_current_routine_index];
    Node.status_ = SUSPEND;
    m_current_routine_index = -1;
    Node.context_->Yield();
    return true;
}



bool YCO_Scheduler::Resume(int index)
{
    if(index >= m_routinue_list.size())
        return false;
    auto context = m_routinue_list[index];

    if(m_routinue_list[index].status_==SUSPEND)
    {
        context.status_=RUNNING;
        m_current_routine_index = index;
        context.context_->Resume();
        return true;
    }
    return false;
}




YRoutine_t YCO_Scheduler::CreateRoutine(context::YRoutineFunc&& func,void* args)
{
    YRoutine_t i = -1;
    if(m_last_routine_index>=0)  //只需要改变空闲槽
    {
        i = m_last_routine_index;
        m_last_routine_index = m_routinue_list[m_last_routine_index].Next_Node;
        m_routinue_list[i].context_->Make(func,args);
    }
    else//需要创建新的
    {
        i = m_routinue_list.size();
        RoutineNode Node;
        Node.context_ = context::YRoutineContext::CreateHandle(m_max_stack_size,func,args,
            std::bind(&YCO_Scheduler::YRoutineDone,this),m_need_memlock_flag); 
        assert(Node.context_);
        m_routinue_list.push_back(Node);
    }

    m_routinue_list[i].status_=SUSPEND; //挂起
    m_routinue_list[i].Next_Node=-1;
    m_routinue_num++;

    return i;
}


