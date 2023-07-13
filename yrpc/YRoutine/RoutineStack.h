/**
 * @file RoutineStack.h
 * @author yqm-307 (979336542@qq.com)
 * @brief 
 * @version 0.1
 * @date 2022-06-04
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#pragma once
#include <memory>
#include <sys/mman.h>   //mmap


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
char* sys_stack_alloc(void* start,size_t len,int opt,int flag,int offset=0);

/**
 * @brief munmap 实现的归还系统内存
 * 
 * @param start 起始地址
 * @param len   内存长度
 * @return int  如果返回0，归还成功；如果为-1，归还失败
 */
int sys_stack_free(void* start,size_t len);




class RoutineStack
{
public:
    explicit
    RoutineStack(const size_t stack_max_size_,const bool memory_protect_ = true);
    ~RoutineStack();
    /* 获取栈顶指针 */
    void* StackTop();   
    /* 返回栈大小 */
    size_t Size();

private:
    void*   m_useable_stack;   //可用栈
    void*   m_stack;
    size_t  m_stacksize;
    bool    m_pagelock_flag;
};


}