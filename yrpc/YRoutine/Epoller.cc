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
bbt::pool_util::IDPool<int,true> Epoller::m_id_pool{65535};


RoutineSocket::RoutineSocket()
    :err("", yrpc::detail::shared::ERRTYPE_NOTHING, 0)
{
}


Epoller::Epoller(size_t stacksize,int maxqueue,bool protect)
    :m_max_size(maxqueue),
    m_runtime(stacksize,protect),
    m_routine_timer(),
    m_epollfd(epoll_create(m_max_size)),
    m_closed(false),
    m_forever_flag(false),
    m_id()
{
    auto [success, id] = m_id_pool.GetID();
    assert(success);
    m_id = id;
    if(m_epollfd < 0)
        FATAL("[YRPC][Epoller::Epoller][%d] error ret : %d", m_id, m_epollfd);
    else
        DEBUG("[YRPC][Epoller::Epoller][%d] epollfd : %d", m_id, m_epollfd);
}

Epoller::~Epoller()
{
    if(m_epollfd>=0)
        close(m_epollfd);
    m_id_pool.ReleaseID(m_id);
}

int Epoller::GetID()
{
    return m_id;
}

void Epoller::AddTask(yrpc::coroutine::context::YRoutineFunc func,void* args)
{   
    yrpc::util::lock::lock_guard<yrpc::util::lock::Mutex> lock(m_lock);
    this->m_pending_tasks.push(std::make_pair(func,args));
}

void Epoller::AddTask_Unsafe(yrpc::coroutine::context::YRoutineFunc func,void* args)
{
    this->m_pending_tasks.push(std::make_pair(func,args));
}


bool Epoller::YieldTask()
{
    return this->m_runtime.Yield(); //当前运行协程 yield
}


bool Epoller::Loop()
{
    DoPendingList();    //先处理待处理任务

    struct epoll_event* events = (struct epoll_event*)malloc(sizeof(struct epoll_event)*m_max_size);

    // while((forever_) || (!runtime_.Empty()))
    while((m_forever_flag) || (!m_runtime.Empty()))
    {
        int nevents = ::epoll_wait(m_epollfd,events,m_max_size,2);   
        //printf("time now: %d-%d\n",clock::minute(),clock::second());
        
        if(nevents > -1)
        {
            //处理epoll中的socket事件
            for(int i = 0;i<nevents;++i)
            {
                RoutineSocket* socket = reinterpret_cast<RoutineSocket*>(events[i].data.ptr);
                socket->eventtype_ = events[i].events;
                m_runtime.Resume(socket->routine_index_);
            }

            if(m_closed)//epoller关闭了
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
    return m_runtime.Size();
}



void Epoller::DoTimeoutTask()
{
    // 定时协程任务 coroutine
    {
        std::vector<NormalTaskPtr> nqueue;
        std::vector<TTaskPtr> queue;
        {// 减小临界区
            lock_guard<Mutex> lock(m_mutex_timer);
            m_routine_timer.GetAllTimeoutTask(queue);
            m_comm_timer.GetAllTimeoutTask(nqueue);
        }

        for (auto && task : queue )   // 处理超时任务
        {
            task->Data()->eventtype_ = EpollREvent_Timeout;
            m_runtime.Resume(task->Data()->routine_index_); // 唤醒超时唤醒协程
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
            lock_guard<Mutex> lock(m_mutex_socket_timer);
            m_socket_timer.GetAllTimeoutTask(queue);
        }
        for (auto && task : queue)   // 处理超时任务
        {
            auto& socket = task->Data();
            socket->eventtype_ = EpollREvent_Timeout;
            DEBUG("[YRPC][Epoller::DoTimeoutTask][%d] socket timeout! \nms: %d\tsockfd: %d\tepollfd: %d",
                y_scheduler_id,
                socket->socket_timeout_ms_,
                socket->sockfd_,
                socket->epollfd_);
            socket->socket_timeout_(socket);    // callback
        }
    }
    {

    }
}

void Epoller::RunForever()
{
    m_forever_flag = true;
}

bool Epoller::QueueFull()
{
    return (m_pending_tasks.size()>=m_max_size);
}


void Epoller::DoPendingList()
{
    while(!m_pending_tasks.empty())
    {
        auto task = m_pending_tasks.front();
        YRoutine_t co_t = m_runtime.Add(task.first,task.second);
        // if( y_scheduler_id == 2 )
        //     DEBUG("[YRPC][TEMP][Epoller::DoPendingList] %d begin rid: %d", y_scheduler_id, co_t);
        if(!m_runtime.Resume(co_t))
            TRACE("Epoller::DoPendingList() m_runtime.Resume() false");
        // if( y_scheduler_id == 2 )
        //     DEBUG("[YRPC][TEMP][Epoller::DoPendingList] %d end rid: %d", y_scheduler_id, co_t);
        m_pending_tasks.pop();
    }
}

void Epoller::ResumeAll(int flag)
{
    std::vector<TTaskPtr> list;
    m_routine_timer.GetAllTask(list);
    for(auto p : list)
    {
        p->Data()->eventtype_ = flag;
        m_runtime.Resume(p->Data()->routine_index_);
    }
}


int Epoller::AddTimer(RoutineSocket* socket,yrpc::util::clock::Timestamp<yrpc::util::clock::ms> ts)
{
    lock_guard<Mutex> lock(m_mutex_timer);
    socket->timetask_ = m_routine_timer.AddTask(ts,socket);
    if(socket->timetask_ != nullptr)
        return 0;
    else
        return -1;
}

int Epoller::AddTimer(TimerTaskFunc&& func,int timeout_ms,int reset_time,int max_trigget_times)
{
    lock_guard<Mutex> lock(m_mutex_timer);
    auto first_trigger_timepoint = yrpc::util::clock::nowAfter<yrpc::util::clock::ms>(yrpc::util::clock::ms(timeout_ms));
    auto task = TimeTask<TimerTaskFunc>::CreateTaskSlotWithSharedOfThis(
                                first_trigger_timepoint,
                                func,
                                reset_time,
                                max_trigget_times);
    return m_comm_timer.AddTask(task);
}


int Epoller::AddSocketTimer(RoutineSocket* socket)
{
    auto timepoint = yrpc::util::clock::now<yrpc::util::clock::ms>()
                    + yrpc::util::clock::ms(socket->socket_timeout_ms_);

    lock_guard<Mutex> lock(m_mutex_socket_timer);
    socket->timetask_ = m_socket_timer.AddTask(timepoint,socket);
    if (socket->timetask_ != nullptr)
        return 0;
    else
        return -1;
}

void Epoller::CancelSocketTimer(RoutineSocket* socket)
{
    m_socket_timer.CancelTask(socket->timetask_);
}


void Epoller::CancelTimer(RoutineSocket*socket)
{
    m_routine_timer.CancelTask(socket->timetask_);
}


YRoutine_t Epoller::GetCurrentRoutine()
{
    return m_runtime.CurrentYRoutine();
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
    socket->m_scheduler = this;
    socket->epollfd_ = this->m_epollfd;
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
    m_suspend_queue.push(GetCurrentRoutine());   //保存当前协程id
    m_runtime.Yield();   //挂起等待下次在loop中被唤醒
}

void Epoller::WakeUpSuspend()
{
    //唤醒所有挂起协程
    while(!m_suspend_queue.empty())
    {
        YRoutine_t a_id = m_suspend_queue.front();
        m_runtime.Resume(a_id);
        m_suspend_queue.pop();
    }
}







int Epoller::ResetSocketTimer(RoutineSocket* socket)
{
    // 重置socket timer，取消之前的socket超时定时器，设置新的socket超时定时器
    this->CancelTimer(socket);  
    return this->AddSocketTimer(socket);
}

int Epoller::GetPollFd()
{
    return m_epollfd;
}


