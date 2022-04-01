#include "../include/Logger.h"


#include <fcntl.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>
#include <stdarg.h>
#include <string.h>

using namespace udp;



Logger* Logger::GetInstance(std::string name)
{
    static Logger* _instance = new Logger(name);
    return _instance;
}


Logger::Logger(std::string name)
{
    filename = name;
    _openfd = open(filename.c_str(),O_RDWR|O_CREAT|O_APPEND,S_IRWXU);  //读写打开文件
    work = [this](){
        while(1)
        {
            static std::string line="";
            if(this->_queue.empty())
                continue;
            this->Dequeue(line);                //取
            write(this->_openfd,line.c_str(),line.size());  //写
            //flush(this->_openfd);
        }
    };
    _writeThread = new std::thread(work);
    //_writeThread->detach();     //没必要
}
//析构显得不是很必要
Logger::~Logger()
{
    close(_openfd); //关闭文件描述符
}

/*
    出队只有一个线程，不需要加锁
*/
bool Logger::Dequeue(std::string& str)
{
    lock_guard<mutex> lock(_mutex);
    if(_queue.size()<=0)
        return false;
    str = _queue.front();   //取队首
    _queue.pop();           //出队
    return true;
}

/*
    可能多个线程调用
*/
void Logger::Enqueue(std::string log)
{
    lock_guard<mutex> lock(_mutex);
    _queue.push(log);
}

void Logger::Log(LOGLEVEL level ,const std::string& str)
{
    if(level < LOG_LEVEL)
        return;
    char log[128];
    int index = 0;
    //当前时间  
    struct timeval tv;
    gettimeofday(&tv, NULL);
    time_t seconds = tv.tv_sec;

    struct tm tm_time;

    gmtime_r(&seconds, &tm_time);


    snprintf(log, 35, "[%4d%02d%02d %02d:%02d:%02d.%06ld]",
                    tm_time.tm_year + 1900, tm_time.tm_mon + 1, tm_time.tm_mday,
                    tm_time.tm_hour, tm_time.tm_min, tm_time.tm_sec, tv.tv_usec);
    

    switch (level)
    {
    case LOGLEVEL::LOG_TRACE :
        strcpy(log+strlen(log),LeveL[0]);
        break;
    case LOGLEVEL::LOG_DEBUG :
        strcpy(log+strlen(log),LeveL[0]);
        break;
    case LOGLEVEL::LOG_INFO :
        strcpy(log+strlen(log),LeveL[0]);
        break;
    case LOGLEVEL::LOG_WARN :
        strcpy(log+strlen(log),LeveL[0]);
        break;
    case LOGLEVEL::LOG_ERROR :
        strcpy(log+strlen(log),LeveL[0]);
        break;
    case LOGLEVEL::LOG_FATAL :
        strcpy(log+strlen(log),LeveL[0]);
        break;
    default:
        break;
    }
    
    strcpy(log+strlen(log),str.c_str());
    strcpy(log+strlen(log),"\n");
    Enqueue(log);


}




std::string udp::format(const char* fmt, ...)
{
    char        data[128];
    size_t      i = 0;
    va_list     ap;



    va_start(ap, fmt);
    vsnprintf(data + i, 128 - i, fmt, ap);
    va_end(ap);

    return std::string(data);

}