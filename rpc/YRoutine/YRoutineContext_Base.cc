#include "YRoutine/YRoutineContext_Base.h"


using namespace yrpc::coroutine::context;


void YRoutineContext_Base::SetYRoutineCreateFunc(YRoutineCreateFunc& func)
{
    routine_create_func_ = func;
}

YRoutineContext_Base* YRoutineContext_Base::Create(size_t init_stack_size,YRoutineFunc main_func,void* args,YRoutineDoneCallback done_func,bool memory_protect)
{
    if(main_func != nullptr)
        return routine_create_func_(init_stack_size,main_func,args,done_func,memory_protect);
    return nullptr;
}

YRoutineCreateFunc YRoutineContext_Base::GetContextCreateFunc()
{
    return routine_create_func_;
}



