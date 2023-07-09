/**
 * @file YRoutineContext.cc
 * @author yqm-307 (979336542@qq.com)
 * @brief 
 * @version 0.1
 * @date 2022-06-04
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#include "YRoutineContext.h"
#include <assert.h>

using namespace yrpc::coroutine::context;
using namespace yrpc::coroutine::detail;

thread_local bool FirstJump = true;


YRoutineContext::YRoutineContext(size_t init_stack_size,YRoutineFunc main_func,void* args,YRoutineDoneCallback done_func,bool memory_protect)
    :m_stack(init_stack_size,memory_protect),
    m_func_main(main_func),m_args(args),
    m_donefunc(done_func)
{
    Make(m_func_main,m_args);
}
YRoutineContext::~YRoutineContext(){}


YRoutineContext_Base* YRoutineContext::CreateHandle(size_t stacksize,YRoutineFunc main_func,void *args,YRoutineDoneCallback done_func,bool memory_protect)
{
    //生命周期由 下一层协程封装管理
    return new YRoutineContext(stacksize,main_func,args,done_func,memory_protect);
}


void YRoutineContext::Make(YRoutineFunc func,void* args)
{
    m_args = args;
    m_func_main = func;

    void* stack_p = (void*)((char*)m_stack.StackTop()+m_stack.Size());    //栈底
    m_context = boost::context::detail::make_fcontext(stack_p,m_stack.Size(),&YRoutineFuncWrapper);


}



bool YRoutineContext::Yield()
{
    boost::context::detail::transfer_t trf;
    trf = boost::context::detail::jump_fcontext(GetMainContext(),&m_context);


    GetMainContext() = trf.fctx;
    return true;
}



bool YRoutineContext::Resume()
{ 
    boost::context::detail::transfer_t trf; // 切换前后上下文

    trf = boost::context::detail::jump_fcontext(m_context,reinterpret_cast<void*>(this));    // main 

    
    //帮助来源协程，保存它的上下文
    auto prev_context_ = trf.data;
    *(void**)prev_context_ = trf.fctx;
    return true;
}

boost::context::detail::fcontext_t& YRoutineContext::GetMainContext()
{
    static thread_local boost::context::detail::fcontext_t Main_context=nullptr; //当前线程协程上下文

    return Main_context;
}

void YRoutineContext::YRoutineFuncWrapper(boost::context::detail::transfer_t t)
{
    assert(t.data!=nullptr);
    YRoutineContext* ts = reinterpret_cast<YRoutineContext*>(t.data);
    
    //一定是从调度协程来的,所以保存调度协程(主协程)上下文
    ts->GetMainContext() = t.fctx;

    ts->m_func_main(ts->m_args);

    if(ts->m_donefunc!=nullptr)
        ts->m_donefunc();
    ts->Yield();    //非对称协程,控制权回到调用协程
}
