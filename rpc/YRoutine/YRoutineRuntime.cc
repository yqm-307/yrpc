#include "YRoutineRuntime.h"



using namespace yrpc::coroutine;

YRoutineRuntime::YRoutineRuntime(size_t stacksize,bool need_memlock)
    :stack_size_(stacksize),
    need_memlock_(need_memlock),r_num_(0),
    last_routine_index_(-1),current_routine_index_(-1)
{
}

YRoutineRuntime::~YRoutineRuntime()
{}


template<class RoutineFunc>
YRoutine_t YRoutineRuntime::Add(RoutineFunc&& func,void* args)
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
            std::bind(&YRoutineRuntime::YRoutineDone,this),need_memlock_); 
        assert(Node.context_);
        r_list_.push_back(Node);
    }

    r_list_[i].status_=SUSPEND; //挂起
    r_list_[i].Next_Node=-1;
    r_num_++;

    return i;
}


//yield之后要切换 当前线程运行的协程
void YRoutineRuntime::YRoutineDone()
{
    if(current_routine_index_ < 0)
        return;
    //修改node的指向，加入到空闲链表
    r_list_[current_routine_index_].Next_Node = last_routine_index_;
    last_routine_index_ = current_routine_index_;
    current_routine_index_ = -1;
    r_list_[current_routine_index_].status_ = DONE;
    r_num_--;
}


bool YRoutineRuntime::Yield()
{
    if(current_routine_index_ < 0)
        return false;   //无效的Yield
    r_list_[current_routine_index_].status_ = SUSPEND;
    r_list_[current_routine_index_].context_->Yield();
    current_routine_index_ = -1;
    return true;
}



bool YRoutineRuntime::Resume(int index)
{
    if(index >= r_list_.size())
        return false;
    auto context = r_list_[index];

    if(r_list_[index].status_==SUSPEND)
    {
        context.status_=RUNNING;
        context.context_->Resume();
        current_routine_index_ = index;
        return true;
    }
    return false;
}






