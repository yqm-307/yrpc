#pragma once
/*
    线程不能自动唤醒自己，而是需要外部调用restart重新进入运行态
    但是可以根据用户设置的 func返回值 来阻塞自己
    例如：用户设置的func逻辑为 从taskqueue中取出一个task然后执行，并返回Running；如果taskqueue为空，就可以让func
    返回Block，这样线程就会挂起了。
*/
#include <thread>
#include <mutex>
#include <condition_variable>   //cv
#include <functional>
#include <atomic>

#include "logger.h"



namespace yrpc::util::threadpool
{
enum ThreadStatus : int32_t
{
        Stop=0,    //停止  
        Running,    //运行
        Blocking    //阻塞  wait
};

class Thread
{
public:
    Thread();
    ~Thread();

    bool Start(std::function<ThreadStatus()> func);
    template<class Data>
    bool Start(std::function<ThreadStatus(Data)> func,Data);
    template<class T,class Data>
    bool Start(T& object,ThreadStatus(T::*func)(Data),Data data);
    void ReStart();
    bool isRun(){ return static_cast<bool>(_run);}     //正在运行
    bool isBlock(){ return static_cast<bool>(_block);} //正在阻塞
protected:
    void Block();                   //要加锁
private:
    std::atomic_bool _run;     //运行控制位
    std::atomic_bool _block;
    std::atomic_bool _running; //是否在运行中
    std::thread _thread;
    std::mutex  _lock;
    std::condition_variable _cv;
};
}

