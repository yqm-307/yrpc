/**
 * @file Epoller.cc
 * @author yqm-307 (979336542@qq.com)
 * @brief 
 * @version 0.1
 * @date 2022-06-04
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#include "Epoller.h"
#include "yrpc/Util/Locker.h"

using namespace yrpc::coroutine::poller;


Epoller::Epoller(size_t stacksize,int maxqueue,bool protect)
    :max_size_(maxqueue),
    runtime_(stacksize,protect),
    timer_(),
    epollfd_(epoll_create(max_size_)),
    close_(false),
    forever_(false)
{
    if(epollfd_ < 0)
        FATAL("[YRPC][Epoller::Epoller] error ret : %d",epollfd_);
    else
        DEBUG("[YRPC][Epoller::Epoller] epollfd : %d",epollfd_);
}

Epoller::~Epoller()
{
    if(epollfd_>=0)
        close(epollfd_);
}


// void Epoller::AddTask(yrpc::coroutine::context::YRoutineFunc&&func,void* args)
// {
//     this->pending_tasks_.push(std::make_pair(func,args));
// }
void Epoller::AddTask(yrpc::coroutine::context::YRoutineFunc func,void* args)
{   
    yrpc::util::lock::lock_guard<yrpc::util::lock::Mutex> lock(m_lock);
    this->pending_tasks_.push(std::make_pair(func,args));
}

void Epoller::AddTask_Unsafe(yrpc::coroutine::context::YRoutineFunc func,void* args)
{
    this->pending_tasks_.push(std::make_pair(func,args));
}


bool Epoller::YieldTask()
{
    return this->runtime_.Yield(); //当前运行协程 yield
}


bool Epoller::Loop()
{
    DoPendingList();    //先处理待处理任务

    struct epoll_event* events = (struct epoll_event*)malloc(sizeof(struct epoll_event)*max_size_);

    // while((forever_) || (!runtime_.Empty()))
    while((forever_) || (!runtime_.Empty()))
    {
        int nevents = ::epoll_wait(epollfd_,events,max_size_,2);   
        //printf("time now: %d-%d\n",clock::minute(),clock::second());
        
        if(nevents > -1)
        {
            //处理epoll中的socket事件
            for(int i = 0;i<nevents;++i)
            {
                RoutineSocket* socket = reinterpret_cast<RoutineSocket*>(events[i].data.ptr);
                socket->eventtype_ = events[i].events;
                runtime_.Resume(socket->routine_index_);
            }

            if(close_)//epoller关闭了
            {
                ResumeAll(EpollREvent_Close);
            }
            WakeUpSuspend();    //唤醒所有挂起任务
            DoPendingList();    //处理积压的待处理任务
            DoTimeoutTask();    //处理超时任务
        }
        else if(errno != EINTR)
        {
            ResumeAll(EpollREvent_Error);
            ERROR("Epoller::Loop() error!");
            return false;
        }


    }
    free(events);
    return true;
    
}

size_t Epoller::Size()
{
    return runtime_.Size();
}



void Epoller::DoTimeoutTask()
{
    // 定时协程任务 coroutine
    {
        std::vector<NormalTaskPtr> nqueue;
        std::vector<TTaskPtr> queue;
        {// 减小临界区
            lock_guard<Mutex> lock(mutex_timer_);
            timer_.GetAllTimeoutTask(queue);
            normal_timer_.GetAllTimeoutTask(nqueue);
        }

        for (auto && task : queue )   // 处理超时任务
        {
            task->Data()->eventtype_ = EpollREvent_Timeout;
            runtime_.Resume(task->Data()->routine_index_); // 唤醒超时唤醒协程
        }
        for (auto && task : nqueue )    // 回调
        {
            assert(task->Data()!=nullptr);
            task->Data()(); //回调
        }
    }

    // 定时套接字任务 callback
    {
        std::vector<TTaskPtr> queue;
        {// 减小临界区
            lock_guard<Mutex> lock(mutex_socket_timer_);
            socket_timer_.GetAllTimeoutTask(queue);
        }
        for (auto && task : queue)   // 处理超时任务
        {
            task->Data()->eventtype_ = EpollREvent_Timeout;
            DEBUG("[YRPC][Epoller::DoTimeoutTask] socket addr : %x  callback addr : %x",task->Data(),task->Data()->socket_timeout_);
            task->Data()->socket_timeout_(task->Data());    // callback
        }
    }
    {

    }
}

void Epoller::RunForever()
{
    forever_ = true;
}

bool Epoller::QueueFull()
{
    return (pending_tasks_.size()>=max_size_);
}


void Epoller::DoPendingList()
{
    decltype(pending_tasks_) queue;

    {// 减少临界区
        yrpc::util::lock::lock_guard<yrpc::util::lock::Mutex> lock(m_lock);
        queue.swap(pending_tasks_);
    }

    while(!queue.empty())
    {
        auto task = queue.front();
        YRoutine_t co_t = runtime_.Add(task.first,task.second);
        if(!runtime_.Resume(co_t))
            TRACE("Epoller::DoPendingList() runtime_.Resume() false");
        queue.pop();
    }
}

void Epoller::ResumeAll(int flag)
{
    std::vector<TTaskPtr> list;
    timer_.GetAllTask(list);
    for(auto p : list)
    {
        p->Data()->eventtype_ = flag;
        runtime_.Resume(p->Data()->routine_index_);
    }
}


int Epoller::AddTimer(RoutineSocket* socket,yrpc::util::clock::Timestamp<yrpc::util::clock::ms> ts)
{
    lock_guard<Mutex> lock(mutex_timer_);
    socket->timetask_ = timer_.AddTask(ts,socket);
    if(socket->timetask_ != nullptr)
        return 0;
    else
        return -1;
}

int Epoller::AddTimer(TimerTaskFunc&& func,int timeout_ms,int reset_time,int max_trigget_times)
{
    lock_guard<Mutex> lock(mutex_timer_);
    auto first_trigger_timepoint = yrpc::util::clock::nowAfter<yrpc::util::clock::ms>(yrpc::util::clock::ms(timeout_ms));
    auto task = TimeTask<TimerTaskFunc>::CreateTaskSlotWithSharedOfThis(
                                first_trigger_timepoint,
                                func,
                                reset_time,
                                max_trigget_times);
    return normal_timer_.AddTask(task);
}


int Epoller::AddSocketTimer(RoutineSocket* socket)
{
    auto timepoint = yrpc::util::clock::now<yrpc::util::clock::ms>()
                    + yrpc::util::clock::ms(socket->socket_timeout_ms_);

    lock_guard<Mutex> lock(mutex_socket_timer_);
    socket->timetask_ = socket_timer_.AddTask(timepoint,socket);
    if (socket->timetask_ != nullptr)
        return 0;
    else
        return -1;
}

void Epoller::CancelSocketTimer(RoutineSocket* socket)
{
    socket_timer_.CancelTask(socket->timetask_);
}


void Epoller::CancelTimer(RoutineSocket*socket)
{
    timer_.CancelTask(socket->timetask_);
}


YRoutine_t Epoller::GetCurrentRoutine()
{
    return runtime_.CurrentYRoutine();
}

RoutineSocket* Epoller::CreateSocket(const int sockfd,const int socket_timeout_ms,const int connect_out_ms,bool nonblocking,const bool nodelay)
{
    RoutineSocket* socket = (RoutineSocket*)calloc(1,sizeof(RoutineSocket));

    if (sockfd >= 0 )
    {
        if(nonblocking)
            util::tcp::SetNonBlockFd(sockfd);
        if (nodelay)
            util::tcp::SetNoDelay(sockfd, nodelay);
    }

    socket->socket_timeout_ms_ = socket_timeout_ms;
    socket->sockfd_ = sockfd;
    socket->connect_timeout_ms_ = connect_out_ms;
    socket->scheduler = this;
    socket->epollfd_ = this->epollfd_;
    socket->event_.data.ptr = socket;
    socket->eventtype_ = 0;
    socket->last_recv_time = 0;
    //socket->
    return socket;
}


void Epoller::DestorySocket(RoutineSocket* freeptr)
{
    free(freeptr);
}


void Epoller::Yield()
{
    suspend_queue_.push(GetCurrentRoutine());   //保存当前协程id
    runtime_.Yield();   //挂起等待下次在loop中被唤醒
}

void Epoller::WakeUpSuspend()
{
    //唤醒所有挂起协程
    while(!suspend_queue_.empty())
    {
        YRoutine_t a_id = suspend_queue_.front();
        runtime_.Resume(a_id);
        suspend_queue_.pop();
    }
}







int Epoller::ResetSocketTimer(RoutineSocket* socket)
{
    // 重置socket timer，取消之前的socket超时定时器，设置新的socket超时定时器
    this->CancelTimer(socket);  
    return this->AddSocketTimer(socket);
}



