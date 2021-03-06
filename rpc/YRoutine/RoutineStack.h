#pragma once
#include <memory>
#include <sys/mman.h>   //mmap


namespace yrpc::coroutine::detail::stack
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

    /**
     * @brief 返回栈顶指针
     * 
     * @return void* 栈顶指针
     */
    void* StackTop();   

    /**
     * @brief 栈大小
     * 
     * @return size_t  
     */
    size_t Size();

private:
    void* useable_stack_;   //可用栈
    void* stack_;
    size_t stacksize_;
    bool is_pagelock_;
};


}