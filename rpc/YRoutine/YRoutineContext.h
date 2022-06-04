#pragma once
#include <boost/context/continuation_fcontext.hpp>
#include "YRoutineContext_Base.h"
#include "RoutineStack.h"


namespace yrpc::coroutine::context
{

using namespace yrpc::coroutine::detail;



/*
*  为什么不保存来源协程的上下文。因为调度不在于jump_fcontext ，选择依靠下一级的Runtime 来进行调度。只需要实现yield和resume即可。
* 关于当前协程的管理和保存都在下一级。
*/
class YRoutineContext:public YRoutineContext_Base
{
public:
    YRoutineContext(size_t init_stack_size,YRoutineFunc main_func,void* args,YRoutineDoneCallback done_func,bool memory_protect);
    ~YRoutineContext();


    static YRoutineContext_Base* CreateHandle(size_t stacksize,YRoutineFunc main_func,void *args,YRoutineDoneCallback done_func,bool memory_protect);

    /**
     * @brief 创建协程上下文环境，并保存到context_ 
     * 
     * @param func 
     * @param args 
     */
    void Make(YRoutineFunc func,void* args);
    
    /**
     * @brief 主动让出，并挂起当前协程
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
     * @brief 用来保存和获取主函数
     * 
     * @return 主函数当前上下文的引用 
     */
    boost::context::detail::fcontext_t& GetMainContext();

private:
    /**
     * @brief 对协程函数和参数的包装
     * @param t fcontext 切换时入参
     */
    static void YRoutineFuncWrapper(boost::context::detail::transfer_t t);

private:
    boost::context::detail::fcontext_t context_;    //协程上下文

    YRoutineFunc main_; 
    void* args_; 

    stack::RoutineStack stack_; //协程栈内存
    YRoutineDoneCallback donefunc_;
};

class YRoutineContextInit
{
    void operator()()
    {
        YRoutineContext::SetYRoutineCreateFunc(YRoutineContext::CreateHandle);
    }
};
}
