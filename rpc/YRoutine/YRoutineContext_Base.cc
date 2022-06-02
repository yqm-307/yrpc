
#include "YRoutineContext_Base.h"


namespace yrpc::coroutine::context
{

YContextCreateFunc YRoutineContext_Base::routine_create_func_ = nullptr;

void YRoutineContext_Base::SetYRoutineCreateFunc(YContextCreateFunc& func)
{
    routine_create_func_ = func;
}

YRoutineContext_Base* YRoutineContext_Base::Create(size_t init_stack_size,YRoutineFunc main_func,void* args,YRoutineDoneCallback done_func,bool memory_protect)
{
    if(routine_create_func_ != nullptr)
        return routine_create_func_(init_stack_size,main_func,args,done_func,memory_protect);
    return nullptr;
}

YContextCreateFunc YRoutineContext_Base::GetContextCreateFunc()
{
    return routine_create_func_;
}

}
