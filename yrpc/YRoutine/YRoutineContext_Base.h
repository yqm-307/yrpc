/**
 * @file YRoutineContext_Base.h
 * @author yqm-307 (979336542@qq.com)
 * @brief 
 * @version 0.1
 * @date 2022-06-04
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#pragma once
#include <iostream>
#include <functional>


namespace yrpc::coroutine::context
{

class YRoutineContext_Base;

typedef std::function<void(void*)> YRoutineFunc;    //协程主函数
typedef std::function<void()> YRoutineDoneCallback; //协程执行时回调
typedef std::function<YRoutineContext_Base*
(size_t,YRoutineFunc,void*,YRoutineDoneCallback,bool)> YContextCreateFunc;


/**
 * @brief 协程上下文虚基类，只用作接口类，由继承者实现功能
 */
class YRoutineContext_Base
{
public:
    YRoutineContext_Base() {}
    virtual ~YRoutineContext_Base() {}

    
    static YRoutineContext_Base* Create(size_t init_stack_size,YRoutineFunc main_func,void* args,YRoutineDoneCallback done_func,bool memory_protect);
    template<class Func = YContextCreateFunc&>
    static void SetYRoutineCreateFunc(Func&& func);

    static YContextCreateFunc GetContextCreateFunc();
    virtual void Make(YRoutineFunc func,void* args)=0;
    virtual bool Yield()=0;
    virtual bool Resume()=0;

private:
    static YContextCreateFunc m_routine_create_func;
};

template<class Func>
void YRoutineContext_Base::SetYRoutineCreateFunc(Func&& func)
{
    m_routine_create_func = func;
}

}