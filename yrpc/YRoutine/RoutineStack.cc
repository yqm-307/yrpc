/**
 * @file RoutineStack.cc
 * @author yqm-307 (979336542@qq.com)
 * @brief 
 * @version 0.1
 * @date 2022-06-04
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#include "RoutineStack.h"
#include "../Util/logger.h"
// #include <YqmUtil/Logger/Logger.h>
#include <unistd.h>
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>


#ifndef MAP_ANONYMOUS
#define MAP_ANONYMOUS MAP_ANON
#endif


#define SYS_LOG(fmt, ... ) \
int save = errno;\
ERROR(fmt,##__VA_ARGS__,errno);\
errno=save;\

namespace yrpc::coroutine::detail
{



/**
 * @brief mmap实现从系统申请内存
 * 
 * @param start 起始地址，如果为NULL，由系统选择起始地址
 * @param len   内存长度，单位byte
 * @param opt   prot权限
 * @param flag  选项
 * @param offset 偏移量，设置为0，即无偏移
 * @return char* 首地址
 */
char* sys_stack_alloc(void* start,size_t len,int opt,int flag,int offset)
{
    flag |= MAP_ANONYMOUS;   //匿名映射
    return (char*)mmap(start,len,opt,flag,-1,offset);
}

/**
 * @brief munmap 实现的归还系统内存
 * 
 * @param start 起始地址
 * @param len   内存长度
 * @return int  如果返回0，归还成功；如果为-1，归还失败
 */
int sys_stack_free(void* start,size_t len)
{
    int ret=0;
    if(ret>munmap(start,len)){
        SYS_LOG("%s , errno : %d",__FUNCTION__)
    }
    return ret; 
}



RoutineStack::RoutineStack(const size_t init_stack_size_,const bool memory_protect_)
    :m_pagelock_flag(memory_protect_),
    m_useable_stack(nullptr),
    m_stack(nullptr),
    m_stacksize(init_stack_size_)
{
    size_t pagesize = getpagesize();  
    // 内存对齐
    if(m_stacksize%pagesize == 0)
        m_stacksize = init_stack_size_ ; 
    else
        m_stacksize = (init_stack_size_/pagesize + 1)*pagesize;

    // 需要保护栈内存
    if(m_pagelock_flag)
    {
        char* stack_ = sys_stack_alloc(NULL,m_stacksize+2*pagesize,
            PROT_READ|PROT_WRITE,MAP_PRIVATE);
        assert(stack_ != nullptr);    

        //内存段加锁
        /*int ret =*/ mprotect(stack_,pagesize,PROT_NONE);
        /* todo 考虑内存申请失败 */
        if(mprotect(stack_,pagesize,PROT_NONE) < 0){
            SYS_LOG("%s , errno : %d",__FUNCTION__)
        }
        if(mprotect((stack_+m_stacksize+pagesize),pagesize,PROT_NONE) < 0){
            SYS_LOG("%s , errno : %d",__FUNCTION__)
        }
        m_useable_stack = stack_+pagesize;   //可访问内存栈   
    }
    else
    {
        char* stack_ = sys_stack_alloc(NULL,m_stacksize,
            PROT_READ|PROT_WRITE,MAP_PRIVATE);
        assert(stack_ != nullptr);
        m_useable_stack = stack_;
    }
}


RoutineStack::~RoutineStack()
{
    int pagesize = getpagesize();
    //解锁内存段
    if(m_pagelock_flag)
    {
        if(mprotect(m_stack,pagesize,PROT_READ|PROT_WRITE) < 0){
            SYS_LOG("%s , errno : %d",__FUNCTION__)
        }
        if(mprotect((char*)m_stack+m_stacksize+pagesize,pagesize,PROT_READ|PROT_WRITE) < 0){
            SYS_LOG("%s , errno : %d",__FUNCTION__)
        }
        assert(sys_stack_free(m_stack,m_stacksize+2*pagesize) == 0);
    }
    else
        assert(sys_stack_free(m_stack,m_stacksize+2*pagesize) == 0);

}




void* RoutineStack::StackTop()
{
    return m_useable_stack;
}

size_t RoutineStack::Size()
{
    return m_stacksize;
}


}
