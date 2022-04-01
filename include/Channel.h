#ifndef EVENT_H
#define EVENT_H

#include <iostream>
#include <sys/epoll.h>

#include <include/Logger.h>
/*
    event 保存fd、fd触发时回调、事件类型(epoll_event)
    接口：
        设置事件类型
        注册对应事件的回调
        
*/
namespace udp
{
class Reactor;

class Channel
{
public:
    typedef std::function<void()> ReadCallback;     //连接建立、有数据
    typedef std::function<void()> WriteCallback;    //发送数据
    typedef std::function<void()> ErrorCallback;    //错误时回调，不必要
    typedef std::function<void()> CloseCallback;    //连接关闭时回调，不必要

    Channel(int fd,Reactor* reactor);
    ~Channel();
    //设置事件类型
    void setRDEvent();
    void setWREvent();

    //注册回调
    void setRDCallback(ReadCallback cb);
    void setWRCallback(WriteCallback cb);
    void setCLSCallback(WriteCallback cb);
    void setErrCallback(ErrorCallback cb);
    
private:
    int event_;     //事件类型
    int fd_;        //持有的文件描述符
    ReadCallback readcb_;
    WriteCallback writecb_;
    ErrorCallback errorcb_;
    CloseCallback closecb_;
};
}

#endif