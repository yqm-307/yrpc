#pragma once
#include <iostream>
#include <functional>


namespace yrpc::coroutine::context
{

class YRoutineContext_Base;

typedef std::function<void(void*)> YRoutineFunc;    //协程主函数
typedef std::function<void()> YRoutineDoneCallback; //协程执行时回调
typedef std::function<YRoutineContext_Base*
(size_t,YRoutineFunc,void*,YRoutineDoneCallback,bool)> YRoutineCreateFunc;


/**
 * @brief 协程上下文虚基类，只用作接口类，由继承者实现功能
 */
class YRoutineContext_Base
{
public:
    YRoutineContext_Base() {}
    virtual ~YRoutineContext_Base() {}

    
    static YRoutineContext_Base* Create(size_t init_stack_size,YRoutineFunc main_func,void* args,YRoutineDoneCallback done_func,bool memory_protect);
    static void SetYRoutineCreateFunc(YRoutineCreateFunc& func);

    static YRoutineCreateFunc GetContextCreateFunc();
    virtual void Make(YRoutineFunc func,void* args);
    virtual bool Yield();
    virtual bool Resume();

private:
    static YRoutineCreateFunc routine_create_func_;
};


}