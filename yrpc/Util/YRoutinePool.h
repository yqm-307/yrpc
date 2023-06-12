/**
 * @file YRoutinePool.h
 * @author your name (you@domain.com)
 * @brief 多线程协程池，协程是独立栈的有栈协程，区分协程绑定栈即可。
 * @version 0.1
 * @date 2022-09-25
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#pragma once
#include "ThreadPool.h"
#include "../YRoutine/Epoller.h"



namespace yrpc::util::YRoutinePool
{



/**
 * @brief 多线程 + 每个线程一个协程调度器，main线程为dispatch线程。其他线程作为run线程。
 * 
 */
class YRoutinePool
{
public:

private:
    void MainDispatcher();  // thread main
    void SubDispatcher();

private:
    yrpc::util::threadpool::ThreadPool<void()> ThreadPool;

    

};



}