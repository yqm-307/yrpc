#ifndef EPOLLER_H
#define EPOLLER_H
#include <sys/epoll.h>
#include <sys/socket.h>
#include <vector>
#include "Channel.h"
/*
    监听所有事件，event触发，执行回调
*/
namespace udp
{
class Epoller:noncopyable
{
public:
    Epoller();
    ~Epoller();
    typedef std::vector<Channel*> ChannelList;

    void wait();
    //向epoll表中添加新的fd。channel中有event，先解析再调用updateChannel
    void updateChannel(Channel& channel);

private:
    void updateChannel(Channel& channel,int event);

private:
    int epollfd_;
    ChannelList channellist_;
    mutex lock; //Epoller实例只有一个，所以可能在多线程环境下被调用，要加锁
};
}

#endif