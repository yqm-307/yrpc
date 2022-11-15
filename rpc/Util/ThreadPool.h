#pragma once
/*
    线程池工作流程：
		池中所有线程都是一个这样的逻辑：不断从taskqueue中取出一个task并执行，如果taskqueue空，则线程休眠。
	线程挂起条件：
		消息队列为空，线程挂起
	阻塞线程的唤醒：
		在插入task的时候，会检测是否有阻塞的线程，如果有就唤醒一个。如果都在运行，就直接入队。
	
*/
#include <functional>
#include <vector>
#include <thread>
#include <atomic>
#include <queue>
#include <condition_variable>   //cv
#include <cassert>
#include "Thread.h"		//封装线程
#include "logger.h"


namespace yrpc::util::threadpool
{

enum ThreadPoolErrnoCode : int32_t
{
	PoolStop=0,
	TaskQueueFull,
	TaskQueueBlock,
	Success
};
typedef std::function<void(uint64_t)> ThreadInitCallback;


template<typename TaskFunc>
class ThreadPool
{
public:
	ThreadPool(int thnum,int maxqueuesize = 65535,const ThreadInitCallback& cb=nullptr);
	//线程池析构并不是热点操作，所以没有性能要求
	~ThreadPool();

	
	/**
	 * @brief 添加一个任务(线程安全的操作)
	 * 
	 * @param task 
	 * @return ThreadPoolErrnoCode 
	 */
	ThreadPoolErrnoCode AddTask(const TaskFunc& task);
	/**
	 * @brief 添加一个任务(线程安全的操作)
	 * 
	 * @param task 
	 * @return ThreadPoolErrnoCode 
	 */
	ThreadPoolErrnoCode AddTask(TaskFunc&& task);

	int RunThreadNum()
	{ return _run_num; }
	int TaskNum()
	{
		std::unique_lock<std::mutex> lock(_lock);
		return _taskqueue.size();
	}
private:
	//终止线程池
	void stop();
	//唤醒一个线程,目前是遍历
	bool WakeUpOne();
private:

	const int _threadnum;					//初始线程数量
	const int _initqueuesize;				//初始队列长度，不是硬性的
	typedef std::vector<Thread*> ThreadList;	//加锁
	std::atomic_bool _pool_is_in_run;		//是否正在运行
	ThreadList _threads;					//线程
	std::queue<TaskFunc> _taskqueue;		//任务队列
	std::mutex _lock;
	std::atomic_int _run_num;				//正在运行数量
	//net::TimerQueue _timer;					//每个线程记录空闲时间
};








//////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////
///////////////////////   私有函数实现   //////////////////////
//////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////


template<typename TaskFunc>
ThreadPool<TaskFunc>::ThreadPool(int thnum,int maxqueuesize,const ThreadInitCallback& cb)
		:_threadnum(thnum),
		_initqueuesize(maxqueuesize),
		_pool_is_in_run(true),
		_threads(thnum,nullptr),
		_run_num(thnum)
	{
		for(int i=0;i<_threadnum;++i)
		{
			_threads[i] = new Thread();
			Thread* ptr = _threads[i];
			//执行任务线程
			ptr->Start([this]()->ThreadStatus{
				if(!_pool_is_in_run)
					return Stop;
				{//进入临界区
					std::unique_lock<std::mutex> lock(_lock);
					if(_taskqueue.empty())	//队列空，阻塞
					{
						--_run_num;		//正在运行线程数减1
						return Blocking;
					}
					else{	//队列非空，取出任务执行
						(_taskqueue.front())();	//task
						_taskqueue.pop();
					}
				}
				return Running;	
			});
		}
	}




template<typename TaskFunc>
ThreadPool<TaskFunc>::~ThreadPool()
{
	stop();
	for(auto ptr : _threads)
	{
		delete ptr;
	}
}

template<typename TaskFunc>
ThreadPoolErrnoCode ThreadPool<TaskFunc>::AddTask(const TaskFunc &task)
{
	if (_pool_is_in_run) // 线程池在运行中
	{
		int queuesize = 0;
		ThreadPoolErrnoCode ret = Success;
		{ // 进入临界区
			std::unique_lock<std::mutex> lock(_lock);
			if (queuesize >= _initqueuesize) // 超出任务数
			{
				ret = TaskQueueFull;
			}
			_taskqueue.push(task); // 插入任务队列

			if (_run_num < _threadnum) // 当前有挂起的线程,唤醒一个线程
			{
				WakeUpOne();
			}
		}

		return ret;
	}
	return PoolStop; // 线程池停止运行
}


template<typename TaskFunc>
ThreadPoolErrnoCode ThreadPool<TaskFunc>::AddTask(TaskFunc &&task)
{
	if (_pool_is_in_run) // 线程池在运行中
	{
		int queuesize = 0;
		ThreadPoolErrnoCode ret = Success;
		{ // 进入临界区
			std::unique_lock<std::mutex> lock(_lock);
			if (queuesize >= _initqueuesize) // 超出任务数
			{
				ret = TaskQueueFull;
			}
			_taskqueue.push(std::move(task)); // 插入任务队列
			if (_run_num < _threadnum)		  // 当前有挂起的线程,唤醒一个线程

				WakeUpOne();
		}

		return ret;
	}
	return PoolStop; // 线程池停止运行
}

template<typename TaskFunc>
void ThreadPool<TaskFunc>::stop()
{
	_pool_is_in_run = false; // 拒绝新的task

	for (size_t i = 0; i < _threadnum;) // 等待所有thread完成当前任务
	{
		if (!_threads[i]->isRun()) // 线程已经stop，下一个
			++i;
		else if (_threads[i]->isBlock()) // 线程阻塞，唤醒
		{
			_threads[i]->ReStart();
		}
	}
}

template<typename TaskFunc>
bool ThreadPool<TaskFunc>::WakeUpOne()
{
	// 遍历所有线程
	assert(_threads[0] != nullptr);
	for (int i = 0; i < _threadnum; ++i)
	{
		if (_threads[i]->isBlock())
		{
			_threads[1]->ReStart();
			++_run_num;
			return true;
		}
	}
	return false; // 唤醒失败
}
}




