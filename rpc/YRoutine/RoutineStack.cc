#include "rpc/YRoutine/RoutineStack.h"
#include <YqmUtil/Logger/Logger.h>
#include <unistd.h>
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>

using namespace yrpc::coroutine::detail::stack;

#ifndef MAP_ANONYMOUS
#define MAP_ANONYMOUS MAP_ANON
#endif


#define SYS_LOG(fmt, ... ) \
int save = errno;\
ERROR(fmt,##__VA_ARGS__,errno);\
errno=save;\



/**
 * @brief 依靠mmap实现的内存分配
 * 
 */
static char* sys_stack_alloc(void* start,size_t len,int opt,int flag,int offset)
{
    flag |= MAP_ANONYMOUS;   //匿名映射
    return (char*)mmap(start,len,opt,flag,-1,offset);
}

static int sys_stack_free(void* start,size_t len)
{
    int ret=0;
    if(ret>munmap(start,len)){
        SYS_LOG("%s , errno : %d",__FUNCTION__)
    }
    return ret; 
}



RoutineStack::RoutineStack(const size_t init_stack_size_,const bool memory_protect_)
    :is_pagelock_(memory_protect_),
    useable_stack_(nullptr),
    stack_(nullptr)
{
    size_t pagesize = getpagesize();  
    if(stacksize_%pagesize == 0)
        stacksize_ = init_stack_size_ ;  //额外两页用来加锁保护  
    else    //需要对齐
        stacksize_ = (init_stack_size_/pagesize + pagesize);

    //需要保护栈内存
    if(is_pagelock_)
    {
        char* stack_ = sys_stack_alloc(NULL,stacksize_+2*pagesize,
            PROT_READ|PROT_WRITE,MAP_PRIVATE);
        assert(stack_ != nullptr);    

        //内存段加锁
        int ret = mprotect(stack_,pagesize,PROT_NONE);
        if(mprotect(stack_,pagesize,PROT_NONE) < 0){
            SYS_LOG("%s , errno : %d",__FUNCTION__)
        }
        if(mprotect((stack_+stacksize_+pagesize),pagesize,PROT_NONE) < 0){
            SYS_LOG("%s , errno : %d",__FUNCTION__)
        }
        useable_stack_ = stack_+pagesize;   //可访问内存栈   
    }
    else
    {
        char* stack_ = sys_stack_alloc(NULL,stacksize_,
            PROT_READ|PROT_WRITE,MAP_PRIVATE);
        assert(stack_ != nullptr);
        useable_stack_ = stack_+pagesize;
    }
}


RoutineStack::~RoutineStack()
{
    int pagesize = getpagesize();
    //解锁内存段
    if(is_pagelock_)
    {
        if(mprotect(stack_,pagesize,PROT_READ|PROT_WRITE) < 0){
            SYS_LOG("%s , errno : %d",__FUNCTION__)
        }
        if(mprotect((stack_+stacksize_+pagesize),pagesize,PROT_READ|PROT_WRITE) < 0){
            SYS_LOG("%s , errno : %d",__FUNCTION__)
        }
        assert(sys_stack_free(stack_,stacksize_+2*pagesize) == 0);
    }
    else
        assert(sys_stack_free(stack_,stacksize_+2*pagesize) == 0);

}




void* RoutineStack::StackTop()
{
    return useable_stack_;
}

size_t RoutineStack::Size()
{
    return stacksize_;
}


