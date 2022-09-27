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


using namespace yrpc::coroutine::poller;


Epoller::Epoller(size_t stacksize,int maxqueue,std::string logpath,bool protect)
    :timer_(),
    max_size_(maxqueue),
    runtime_(stacksize,protect),
    epollfd_(epoll_create(max_size_)),
    close_(false),
    forever_(false)
{
    yrpc::util::logger::Logger::GetInstance(logpath);  //打开日志

    if(epollfd_ < 0)
        FATAL("Epoller::Epoller() epoll_create() error ret : %d",epollfd_);
    else
        DEBUG("Epoller::Epoller() epoll_create() epollfd : %d",epollfd_);
}

Epoller::~Epoller()
{
    if(epollfd_>=0)
        close(epollfd_);
}


void Epoller::AddTask(yrpc::coroutine::context::YRoutineFunc&&func,void* args)
{
    this->pending_tasks_.push(std::make_pair(func,args));
}
void Epoller::AddTask(yrpc::coroutine::context::YRoutineFunc&func,void* args)
{   
    this->pending_tasks_.push(std::make_pair(func,args));
}

bool Epoller::YieldTask()
{
    this->runtime_.Yield(); //当前运行协程 yield
}


bool Epoller::Loop()
{
    DoPendingList();    //先处理待处理任务

    struct epoll_event* events = (struct epoll_event*)malloc(sizeof(struct epoll_event)*max_size_);

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
        else if(errno == EINTR)
        {
            ResumeAll(EpollREvent_Error);
            ERROR("Epoller::Loop() error!");
            return false;
        }


    }
    free(events);
    return true;
    
}

void Epoller::DoTimeoutTask()
{
    while(true)
    {
        TTaskPtr timetask = timer_.GetATimeoutTask();   //弹出超时任务
        if(timetask == nullptr)   //当前没有超时事件了
            break;
        timetask->data()->eventtype_ = EpollREvent_Timeout;
        runtime_.Resume(timetask->data()->routine_index_);    //执行
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
    while(!pending_tasks_.empty())
    {
        auto task = pending_tasks_.front();
        YRoutine_t co_t = runtime_.Add(task.first,task.second);
        if(!runtime_.Resume(co_t))
            TRACE("Epoller::DoPendingList() runtime_.Resume() false");
        pending_tasks_.pop();
    }
}

void Epoller::ResumeAll(int flag)
{
    std::vector<RoutineSocket*> list;
    timer_.GetAllTask(list);
    for(auto p : list)
    {
        p->eventtype_ = flag;
        runtime_.Resume(p->routine_index_);
    }
}


int Epoller::AddTimer(RoutineSocket* socket,yrpc::util::clock::Timestamp<yrpc::util::clock::ms> ts)
{
    socket->timetask_ = timer_.AddTask(ts,socket);
    if(socket->timetask_ != nullptr)
        return 0;
    else
        return -1;
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



