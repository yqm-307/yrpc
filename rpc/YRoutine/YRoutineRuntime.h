#pragma once

#include <stdlib.h>
#include <vector>
#include "YRoutineContext.h"   //协程上下文



namespace yrpc::coroutine
{

/**
 * @brief 协程运行时状态
 * 
 */
enum YRoutineRuntimeStatus
{//基础状态：运行、阻塞、就绪。就是调度的基础标识
    RUNNING=0,
    SUSPEND,
    BLOCK,
    DONE    
};


/**
 * @brief 协程节点，利用数组和保存下一节点下标来映射为一个功能类似为链表的结构
 * 
 */
struct RoutineNode
{
    context::YRoutineContext* context_ = nullptr;
    int Next_Node=-1;
    YRoutineRuntimeStatus status_;
};
typedef int YRoutine_t;



/**
 * @brief 运行线程内管理多个协程及调度(yield、resume)
 */
class YRoutineRuntime
{
public:
    /**
     * @param stacksize_ 调度器中，协程栈大小
     * @param need_memlock_ 是否需要页锁保护协程栈
     */
    YRoutineRuntime(size_t stacksize_,bool need_memlock_);
    ~YRoutineRuntime();

    /**
     * @brief 添加一个协程
     * 
     * @tparam  
     * @param func std::function<void(void*)>
     * @param args 参数
     * @return YRoutine_t 协程句柄
     */
    template<class RoutineFunc = context::YRoutineFunc> //只是为了好看
    YRoutine_t Add(RoutineFunc&& func,void* args);

    /**
     * @brief 挂起当前运行协程
     * 
     * @return true 挂起成功
     * @return false 无效的Yield，不意味失败，可能是当前线程没有正在运行的Routine
     */
    bool Yield();

    /**
     * @brief 唤醒指定协程
     * 
     * @param index 
     * @return true 唤醒成功
     * @return false 唤醒失败
     */
    bool Resume(int index);


    /**
     * @brief 是否有协程
     * 
     * @return true 没有未完成协程
     * @return false 有未完成协程
     */
    bool Empty(){return !r_num_;}

    /**
     * @brief 获取当前协程数量
     * 
     * @return YRoutine_t 协程数量
     */
    size_t Size(){return r_num_;}


    /**
     * @brief 获取正在运行的协程句柄
     * 
     * @return int 
     */
    YRoutine_t CurrentYRoutine(){return current_routine_index_;}

private:
    /**
     * @brief 协程运行结束时回调，改变运行时协程和协程调用栈
     */
    void YRoutineDone();    

private:
    typedef std::vector<RoutineNode> YRoutineList;

    size_t stack_size_;
    YRoutineList r_list_;       
    int current_routine_index_; //正在运行的协程
    int last_routine_index_;    //记录空闲协程
    
    //r_list 并不是 链表，只是数组，每个元素只是一个槽位，
    //可以参考我写的定时器队列，也是运用的这个原理，所
    //以需要有专门的一个记录当前协程数量的参数
    //优点1: 对象可以重复利用
    //优点2: 有数组的效率和链表的灵活
    //缺点: 因为内部复杂的映射关系，所以没法clear来缩容。如果协程最大可能出现100万个，哪怕只有一次，数组都有这么大。
    //解决方案，主动降容，或者根据r_num_ ，在多次判断r_num_使用了不到r_list_ size 的25，就缩容。
    int r_num_;                 
    bool need_memlock_;
};




}