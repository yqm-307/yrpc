#pragma once
#include <boost/context/continuation_fcontext.hpp>
#include "rpc/YRoutine/YRoutineContext_Base.h"
#include "rpc/YRoutine/RoutineStack.h"


namespace yrpc::coroutine::context
{

using namespace yrpc::coroutine::detail;



class YRoutineContext:YRoutineContext_Base
{
public:
    YRoutineContext(size_t init_stack_size,YRoutineFunc main_func,void* args,YRoutineDoneCallback done_func,bool memory_protect);
    ~YRoutineContext();


    static YRoutineContext* CreateHandle(size_t stacksize,YRoutineFunc main_func,void *args,YRoutineDoneCallback done_func,bool memory_protect);

    /**
     * @brief 创建协程上下文环境，并保存到context_ 
     * 
     * @param func 
     * @param args 
     */
    void Make(YRoutineFunc func,void* args);
    /**
     * @brief 
     * 
     * @return true 
     * @return false 
     */
    bool Yield() override;
    /**
     * @brief 主动将当前线程的上下文与context_切换
     * 
     * @return true 
     * @return false 
     */
    bool Resume() override;
    
    /**
     * @brief 获取当前线程上下文
     * 
     * @return 线程当前上下文的引用 
     */
    boost::context::detail::fcontext_t& GetCurrentContext();

private:
    static void YRoutineFuncWrapper(boost::context::detail::transfer_t t);

private:
    boost::context::detail::fcontext_t context_;    //协程上下文

    YRoutineFunc main_; 
    void* args_; 

    stack::RoutineStack stack_; //协程栈内存
    YRoutineDoneCallback donefunc_;
};


}