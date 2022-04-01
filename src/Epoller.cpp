#include "include/Epoller.h"

using namespace udp;


Epoller::Epoller()
    :epollfd_(epoll_create1(EPOLL_CLOEXEC))
{
    
}