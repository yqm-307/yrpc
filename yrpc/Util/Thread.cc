#include "Thread.h"
#include <unistd.h>

using namespace yrpc::util::threadpool;

Thread::Thread()
    :_run(false),
    _block(false),
    _running(false)
{
}

/*
    设置100ms超时时间，如果100ms没有处理完请求，则join
*/
Thread::~Thread()
{
    _run = false;   //改变运行标志
    int time_now=0;
    int time_stop = 100;    //超时时间
    do{
        usleep(10);
        time_now+=10;
        if(!_running)   //不在运行中就退出
            break;        
    }while(time_now<time_stop);

    while(!_running && _thread.joinable())
        _thread.join(); //防止主线程提前退出，导致程序崩溃
}


//启动一个线程，在循环中，可能阻塞
bool Thread::Start(std::function<ThreadStatus()> func)
{
    if(nullptr == func)
        return false;
    
    _thread = std::thread([this,func](){

        _running = true;    //运行中
        _run = true;        //运行控制位
        while(_run)
        {
            switch (func()) //调用func
            {
            case Blocking:  //阻塞  
                Block();    //阻塞在这里
                break;
            case Stop:     //线程停止
                _run = false;
                break;
            default:        //Running 
                break;
            }
        }
        _running = false;
    });
    return true;
}

template<class Data>
bool Thread::Start(std::function<ThreadStatus(Data)> func,Data data)
{
    if(nullptr == func)
        return false;
    return Start([this,&data,func](){
        return func(data);
    });
}

//将object传入， data是task的入参
template<class T,typename Data>
bool Thread::Start(T& object,ThreadStatus(T::*func)(Data),Data data)
{
    if(nullptr == func)
        return false;

    //Thread执行 object.*func
    return Start([this,&object,data,func](){
        //data被引用捕获
        return (object.*func)(data);
    }
    );
}

//唤醒当前线程
void Thread::ReStart()
{
    std::unique_lock<std::mutex> lock(_lock);
    //进入ReStart
    if(_block)
    {
        _cv.notify_all();   //唤醒当前线程
    }
}

//阻塞当前线程
void Thread::Block()
{
    std::unique_lock<std::mutex> lock(_lock);
    _block=true;    //开始阻塞
    _cv.wait(lock);             
    _block=false;   //阻塞结束
}

