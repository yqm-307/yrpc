#include "YRoutineContext.h"
//#include "rpc/YRoutine/YRoutineContext.h"

using namespace yrpc::coroutine::context;
using namespace yrpc::coroutine::detail;

thread_local bool FirstJump = true;


YRoutineContext::YRoutineContext(size_t init_stack_size,YRoutineFunc main_func,void* args,YRoutineDoneCallback done_func,bool memory_protect)
    :stack_(init_stack_size,memory_protect),
    main_(main_func),args_(args),
    donefunc_(done_func)
{
    Make(main_,args_);
}
YRoutineContext::~YRoutineContext(){}


YRoutineContext_Base* YRoutineContext::CreateHandle(size_t stacksize,YRoutineFunc main_func,void *args,YRoutineDoneCallback done_func,bool memory_protect)
{
    //生命周期由 下一层协程封装管理
    return new YRoutineContext(stacksize,main_func,args,done_func,memory_protect);
}


void YRoutineContext::Make(YRoutineFunc func,void* args)
{
    args_ = args;
    main_ = func;

    void* stack_p = (void*)((char*)stack_.StackTop()+stack_.Size());    //栈底
    context_ = boost::context::detail::make_fcontext(stack_p,stack_.Size(),&YRoutineFuncWrapper);
}



bool YRoutineContext::Yield()
{
    boost::context::detail::transfer_t trf;
    trf = boost::context::detail::jump_fcontext(GetMainContext(),&context_);
    //只有 子协程 会调用yield，那么返回也是子协程，来源是主协程
    GetMainContext() = trf.fctx;
    return true;
}



bool YRoutineContext::Resume()
{
    boost::context::detail::transfer_t trf;
    trf = boost::context::detail::jump_fcontext(context_,reinterpret_cast<void*>(this));
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

//问题在于主函数不走函数包装器，如果只有协程之间切换，倒是没什么问题
void YRoutineContext::YRoutineFuncWrapper(boost::context::detail::transfer_t t)
{
    assert(t.data!=nullptr);
    YRoutineContext* ts = reinterpret_cast<YRoutineContext*>(t.data);
    
    //第一次resume一定是冲main来的，需要保存main的上下文状态
    ts->GetMainContext() = t.fctx;

    ts->main_(ts->args_);
    //回调通知调度器，执行完毕
    if(ts->donefunc_!=nullptr)
        ts->donefunc_();
    //执行完成，yield回到主线程
    ts->Yield();
}




/**
 * @biref 跳转到目标上下文
 * @param to 跳转的目标上下文
 * @param vp resume是参数，yield是返回值
 * @return 当前上下文

transfer_t BOOST_CONTEXT_CALLDECL jump_fcontext( fcontext_t const to, void * vp);
*/



/**
 * @biref 初始化执行环境上下文
 * @param sp 栈底地址
 * @param size 栈空间的大小
 * @param fn 入口函数
 * @return 返回初始化完成后的执行环境上下文

fcontext_t BOOST_CONTEXT_CALLDECL make_fcontext( void * sp, std::size_t size, void (* fn)( transfer_t) );
*/