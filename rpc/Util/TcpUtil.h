#pragma once
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include "logger.h"

namespace yrpc::util::tcp
{

static int SetNonBlockFd(int fd)
{
    int oldopt = ::fcntl(fd,F_GETFL);
    int ret = fcntl(fd, F_SETFL, oldopt | O_NONBLOCK);
    if(ret < 0)
    {
        ERROR("SetNonBlockFd() error!");
        return -1;
    }
    return oldopt;
}
static int SetNoDelay(int fd,bool flag)
{
    int tmp = flag ? 1:0;

    int ret = setsockopt(fd,IPPROTO_TCP,TCP_NODELAY,&tmp,sizeof(tmp));

    if(ret != 0)
        ERROR("SetNoDelay() error!");
    return ret==0;
}

static int GetSockErrCode(int socket)
{
    int retcode;
    socklen_t len = sizeof(retcode);
    int n = getsockopt(socket,SOL_SOCKET,SO_ERROR,&retcode,&len);
    if(n<0)
        return n;
    else
        return retcode;
}

}



