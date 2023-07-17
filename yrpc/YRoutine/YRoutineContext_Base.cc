/**
 * @file YRoutineContext_Base.cc
 * @author yqm-307 (979336542@qq.com)
 * @brief 
 * @version 0.1
 * @date 2022-06-04
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#include "YRoutineContext_Base.h"


namespace yrpc::coroutine::context
{

YContextCreateFunc YRoutineContext_Base::m_routine_create_func = nullptr;



YRoutineContext_Base* YRoutineContext_Base::Create(size_t init_stack_size,YRoutineFunc main_func,void* args,YRoutineDoneCallback done_func,bool memory_protect)
{
    if(m_routine_create_func != nullptr)
        return m_routine_create_func(init_stack_size,main_func,args,done_func,memory_protect);
    return nullptr;
}

YContextCreateFunc YRoutineContext_Base::GetContextCreateFunc()
{
    return m_routine_create_func;
}

}
