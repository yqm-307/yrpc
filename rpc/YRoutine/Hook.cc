/**
 * @file Hook.cc
 * @author yqm-307 (979336542@qq.com)
 * @brief 对于如何hook，我这里使用的思路是对于一个系统调用，如果hook之后行为应该不变，所以不管成功或者失败我们立即返回，
 * 但是如何利用协程提高效率呢？
 * 
 *  答：设置socket为 non-block 这样如果是慢系统调用也会立即返回，就可以判断为需要长时间阻塞去完成（我们守着等结果没有意义）
 *  立即使用epoll-scheduler管理，注册socket协程，如果epoll触发，我们就知道socket上的io事件完成了，就可以根据epoll_event的
 *  data指针来 resume 继续运行之前阻塞的位置。
 * @version 0.1
 * @date 2022-06-08
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#include "./Hook.h"
#include <unistd.h>



namespace yrpc::socket
{
#define YRAfter(DDtimeout_ms) (yrpc::util::clock::nowAfter<yrpc::util::clock::ms,yrpc::util::clock::Timestamp<yrpc::util::clock::ms>>(yrpc::util::clock::ms(DDtimeout_ms)))


int YRPoll(Socket* socket,int events,int* revents,int timeout_ms)
{
    if(socket == nullptr)
        return -1;
    

    socket->routine_index_ = socket->scheduler->GetCurrentRoutine();
    socket->event_.events = events; //新事件的events

    if(timeout_ms < 0)
        socket->scheduler->AddTimer(socket,YRAfter(INT32_MAX));
    else
        socket->scheduler->AddTimer(socket,YRAfter(timeout_ms));

    //协程中解决
    epoll_ctl(socket->epollfd_,EPOLL_CTL_ADD,socket->sockfd_,&socket->event_);
    socket->scheduler->YieldTask(); //处理协程任务
    epoll_ctl(socket->epollfd_,EPOLL_CTL_DEL,socket->sockfd_,&socket->event_);
    socket->scheduler->CancelTimer(socket);

    *revents = socket->eventtype_;  //保存 自定义 事件错误码

    //处理错误码
    if(*revents>0)
    {
        if(*revents & events)
            return 1;
        else
        {
            errno = EINVAL;
            return 0;
        }
    }else if((*revents) == yrpc::coroutine::poller::EpollREvent_Timeout)
    {
        errno = ETIMEDOUT;  //超时
        return  0;
    }else if(*revents == yrpc::coroutine::poller::EpollREvent_Error)
    {
        errno = ECONNREFUSED;   //连接拒绝
        return -1;
    }else   //close
    {
        errno = 0;
        return -1;
    }
    
    return -1;
}


//客户端调用，作为tcp握手开始，所以是写事件
int YRConnect(Socket &socket, const struct sockaddr *addr, socklen_t addrlen)
{

    int ret = ::connect(socket.sockfd_,addr,addrlen);

    if(0 != ret)
    {
        if(EAGAIN != errno && EINPROGRESS != errno)
            return -1;
        else    //非阻塞错误
        {
            int revents = 0;
            //交给epoll、协程去处理
            int nfds = YRPoll(&socket,EPOLLOUT,&revents,socket.connect_timeout_ms_);
            if(nfds > 0)
                return 0;
            else
                return -1;
        }
    }//成功、失败都不会在epoll中处理，只有非阻塞错误，才需要在epoll中等待处理

    return ret;
}





//服务端调用，作为tcp握手完毕接受新的套接字，是读事件
int YRAccept(Socket &listenfd, struct sockaddr *addr, socklen_t *addrlen)
{
    int ret = ::accept(listenfd.sockfd_,addr,addrlen);

    if(ret < 0)
    {
        if(EAGAIN != errno && EINPROGRESS != errno)
            return -1;
        else
        {//可能listen队列没有新连接，加入epoll，让出cpu，等待listenfd可读
            int revent =0;
            //加入epoll，让出cpu，等listenfd可读
            int nfds = YRPoll(&listenfd,EPOLLIN,&revent,-1);
            //可读
            if(nfds >= 0)//在此accept
                return accept(listenfd.sockfd_,addr,addrlen);
            else
                return -1;
        }
    }//成功or失败直接返回
    return ret;
}

int YRCreateListen(int* fd,int port)
{
    int listenfd = ::socket(AF_INET,SOCK_STREAM,0);
    if(listenfd < 0)
        ERROR("YRCreateListen() socket, create listenfd error!");
    
    struct sockaddr_in servaddr;
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(port);
    servaddr.sin_addr.s_addr = INADDR_ANY;

    int flag = 1;
    int ret = ::setsockopt(listenfd,SOL_SOCKET,SO_REUSEADDR,&flag,sizeof(flag));
    if(ret < 0){
        ERROR("YRCreateListen() setsockopt, set socket option error!");
        ret = -1;
    }
    //bind
    ret = ::bind(listenfd,(sockaddr*)&servaddr,sizeof(servaddr));
    if(ret < 0){
        ERROR("YRCreateListen() bind, listenfd bind sockaddr error!");
        ret = -2;
    }
    //listen
    ret = ::listen(listenfd,1024);  //参数二：阻塞因子，不是绝对控制就绪队列长度的指标，不同标准之下都有自己跌定义
    if(ret < 0){
        ERROR("YRCreateListen() listen,");
        ret = -3;
    }
    //错误，关闭fd
    if( ret < 0 && listenfd>=0){
        close(listenfd);
    }


    *fd = listenfd;  //入参的返回值形式
    return ret;
}


//读事件
ssize_t YRRecv(Socket &socket, void *buf, size_t len, const int flags)
{
    int nbytes = ::recv(socket.sockfd_,buf,len,flags);
    if(nbytes<0)
    {
        if(errno != EAGAIN)
            return -1;
        else
        {
            int revents=0;
            if(YRPoll(&socket,EPOLLIN,&revents,socket.socket_timeout_ms_) >= 0) //让出cpu，直到可读事件触发
                nbytes = ::recv(socket.sockfd_,buf,len,flags);
            else
                nbytes = -1;
        }
    }
    else
    {
        socket.last_recv_time = yrpc::util::clock::now<yrpc::util::clock::ms>().time_since_epoch().count();
    }

    return nbytes;
}

//读事件
ssize_t YRRead(Socket &socket, void *buf, size_t len)
{
    int nbytes = ::read(socket.sockfd_,buf,len);
    if(nbytes<0)
    {
        if(errno!=EAGAIN)
            return -1;
        else
        {
            int re =0;
            if(YRPoll(&socket,EPOLLIN,&re,socket.socket_timeout_ms_)>=0)
                nbytes = ::read(socket.sockfd_,buf,len);
            else
                nbytes=-1;
        }
    }
    else
    {
        socket.last_recv_time = yrpc::util::clock::now<yrpc::util::clock::ms>().time_since_epoch().count();
    }

    return nbytes;
}

//写事件
ssize_t YRSend(Socket &socket, const void *buf, size_t len, const int flags)
{
    int nbytes = ::send(socket.sockfd_,buf,len,flags);
    if(nbytes<0 && errno == EAGAIN)
    {
        int rev = 0;
        if(YRPoll(&socket,EPOLLOUT,&rev,-1)>=0)
            nbytes = ::send(socket.sockfd_,buf,len,flags);
        else
            nbytes = -1;
    }
    return nbytes;
}


ssize_t YRWrite(Socket &socket, const void *buf, size_t len)
{
    int nbytes = ::write(socket.sockfd_,buf,len);
    if(nbytes<0 && errno == EAGAIN)
    {
        int rev = 0;
        if(YRPoll(&socket,EPOLLOUT,&rev,-1)>=0)
            nbytes = ::write(socket.sockfd_,buf,len);
        else
            nbytes = -1;
    }
    return nbytes;
}


int YRClose(Socket &socket)
{
    if(socket.timetask_.use_count() >= 2)
        DEBUG("YRClsoe() socket use_count >= 2,memory leak!");
    socket.timetask_ = nullptr; //释放最后的引用计数
    if(socket.sockfd_>=0)
    {
        return ::close(socket.sockfd_);
    }
    return -1;
}

void YRSetConnectTimeout(Socket &socket, const int connect_timeout_ms)
{
    socket.connect_timeout_ms_ = connect_timeout_ms;
}

void YRSetSocketTimeout(Socket &socket, const int socket_timeout_ms)
{
    socket.socket_timeout_ms_ = socket_timeout_ms;
}

void YRWait(Socket &socket, const int timeout_ms)
{
    socket.routine_index_ = socket.scheduler->GetCurrentRoutine();
    socket.scheduler->AddTimer(&socket,YRAfter(timeout_ms));
    socket.scheduler->YieldTask();
}


int YRSocketFd(Socket &socket)
{
    return socket.sockfd_;
}

Socket *YRCreateSocket()
{
    return (Socket*)calloc(1,sizeof(Socket));
}


// void YRSetArgs(Socket &socket, void *args)
// {
//     socket.args = args;
// }

// void *YRGetArgs(Socket &socket)
// {
//     return socket.args;
// }

// void YRLazyDestory(Socket &socket)
// {
//     socket.routine_index_ = -1;
// }

// bool YRDestory(Socket &socket)
// {
//     socket.routine_index_ = -1;
//     return true;
// }





Epoll_Cond_t::Epoll_Cond_t()
{
    pipe_fds_[0] = pipe_fds_[1] = -1;
}

Epoll_Cond_t::~Epoll_Cond_t()
{
    scheduler_->DestorySocket(socket_);
    if (pipe_fds_[0] != -1) {
        close(pipe_fds_[0]);
    }
    if (pipe_fds_[1] != -1) {
        close(pipe_fds_[1]);
    }
}

int Epoll_Cond_t::Init(yrpc::coroutine::poller::Epoller* scheduler,int timeout)
{
    int ret = ::pipe(pipe_fds_);
    if (0 != ret)
        return ret;
    fcntl(pipe_fds_[1], F_SETFL, O_NONBLOCK);
    socket_ = scheduler->CreateSocket(pipe_fds_[0], timeout, -1, true,false);

    return 0;
}

int Epoll_Cond_t::Wait()
{
    char buf;
    ssize_t read_len = YRRead(*socket_, (void *)&buf, 1);
    if (0 > read_len) {
        ERROR("Epoll_Cond_t::Wait() YRRead errno %d ,sockfd: %d", errno,socket_->sockfd_);
        return -1;
    }
    data_ = nullptr;
    return 0;
}

int Epoll_Cond_t::Notify()
{
    //发送 1 ，唤醒read协程
    ssize_t write_len = write(pipe_fds_[1], (void *)"1", 1);    
    if (0 > write_len) {
        ERROR("Epoll_Cond_t::Notify() write errno %d ,sockfd: %d", errno,socket_->sockfd_);
        return -1;
    }
    return 0;
}


//如何实现协程的sleep,如果是注册在pendinglist 中的任务，是没有办法让他不执行的，所以要将任务切换到timeoutlist中，这样就不会在loop中执行，且超时resume
int YRSleep(yrpc::coroutine::poller::Epoller *poll, int sleep_ms)
{
    int ret{-1};
    yrpc::coroutine::poller::RoutineSocket *socket = poll->CreateSocket(-1);
    do
    {

        if (socket == nullptr)
        {
            ret = -2;
            break;
        }
        socket->routine_index_ = socket->scheduler->GetCurrentRoutine();

        if (sleep_ms < 0)
        {
            ret = -3;
            break;
        }
        else
        {
            auto k = YRAfter(sleep_ms);
            socket->scheduler->AddTimer(socket, k);
        }
        // 等待超时，resume
        socket->scheduler->YieldTask(); // 处理协程任务

        if (socket->eventtype_ == yrpc::coroutine::poller::EpollREvent_Timeout)
        {
            ret = 0;
            break;
        }
        else
            ret = -4;
    } while (0);


    return ret;
}

#ifdef YRAfter
#undef YRAFter
#endif

}