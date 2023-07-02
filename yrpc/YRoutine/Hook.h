#pragma once
#include <iostream>
#include <netinet/in.h>
#include <sys/epoll.h>
#include <sys/fcntl.h>
#include <arpa/inet.h>

#include "./Epoller.h"

namespace yrpc::socket
{
typedef yrpc::coroutine::poller::RoutineSocket Socket;


class Epoll_Cond_t final
{
public:
    Epoll_Cond_t();
    ~Epoll_Cond_t();

    int Init(yrpc::coroutine::poller::Epoller* scheduler,int timeout_ms=-1);
    int Wait();
    int Notify();
private:
    yrpc::coroutine::poller::Epoller *scheduler_{nullptr};
    Socket *socket_{nullptr};
    int pipe_fds_[2];
    void *data_{nullptr};
};

//int YHook_Poll(Socket** socket,int events,int* revents,yrpc::util::clock::Timestamp timeout_ms);

int YRPoll(Socket* socket,int events,int* revents,int timeout_ms);

int YRConnect(Socket &socket, const struct sockaddr *addr, socklen_t addrlen);

int YRAccept(Socket &socket, struct sockaddr *addr, socklen_t *addrlen);

int YRCreateListen(int* fd,int port);

ssize_t YRRecv(Socket &socket, void *buf, size_t len, const int flags);

ssize_t YRRead(Socket &socket, void *buf, size_t len);

ssize_t YRSend(Socket &socket, const void *buf, size_t len, const int flags);

ssize_t YRWrite(Socket &socket, const void *buf, size_t len);

int YRClose(Socket &socket);

void YRSetConnectTimeout(Socket &socket, const int connect_timeout_ms);

void YRSetSocketTimeout(Socket &socket, const int socket_timeout_ms);

int YRSocketFd(Socket &socket);

Socket *YRCreateSocket();

void YRWait(Socket &socket, const int timeout_ms);

int YRSleep(yrpc::coroutine::poller::Epoller* poll, int sleep_ms); //挂起当前协程指定时间

Socket::RawPtr CreateSocket(const int sockfd, yrpc::coroutine::poller::Epoller* poll, int poll_fd, int timeout_ms=5000, int connect_out_ms=3000,bool noblocking = true, bool nodelay=true);

void DestorySocket(Socket::RawPtr socket);

}

