// #include "./logger.h"
// #include "./Clock.h"

// #include <fcntl.h>
// #include <unistd.h>
// #include <time.h>
// #include <sys/time.h>
// #include <stdarg.h>
// #include <string.h>
// #include <assert.h>

// using namespace yrpc::util::logger;



// static const char* LeveL[6]{
//     " [TRACE] ",
//     " [DEBUG] ",
//     " [INFO] ",
//     " [WARN] ",
//     " [ERROR] ",
//     " [FATAL] ",
// };


// Logger* Logger::GetInstance(std::string name)
// {
//     static Logger* _instance = new Logger(name);
//     return _instance;
// }

// #ifdef YNET_LOG_BUFFER
// Logger::Logger(std::string name)
//     :_pendingwriteindex(0),
//     _nowindex(0),
//     _nowsize(ARRAY_NUM)
// {
//     using namespace std::chrono;
//     for(int i=0;i<ARRAY_NUM;++i)
//     {
//         _buffers.push_back(std::pair<int,char*>((i+1)%ARRAY_NUM,new char[ARRAY_SIZE]));
//     }
    

//     filename = name;
//     _openfd = open(filename.c_str(),O_RDWR|O_CREAT|O_APPEND,S_IRWXU);  //读写打开文件

//     /*如果有buffer满，则唤醒IO线程，将数据写入磁盘，如果写入完毕就进入休眠，并定时苏醒*/
//     auto work = [this](){
//         while(1)
//         {
//             std::unique_lock<std::mutex> loc(_condlock);
            
//             _cond.wait_for(loc,100ms);

//             //先写满的buffer
//             while(true)
//             {
//                 if(hasfulled())
//                 {
//                     assert(flushbuffer(this->_pendingwriteindex) >= 0);
//                 }
//                 else
//                     break;
//             }
//             //写入当前的
//             assert(flushbuffer(_nowindex)>=0);
//         }
//     };
//     _writeThread = new std::thread(work);
// }
// #else
// Logger::Logger(std::string name)
// {
//     filename = name;
//     _openfd = open(filename.c_str(),O_RDWR|O_CREAT|O_APPEND,S_IRWXU);  //读写打开文件
    
//     //写函数
//     _writeThread = new std::thread([this](){
//         while(1)
//         {
//             static std::string line="";
//             if(this->_queue.empty())
//                 continue;
//             this->Dequeue(line);                //取
//             write(this->_openfd,line.c_str(),line.size());  //写

//         }
//     });
// }
// #endif



// Logger::~Logger()
// {
// #ifdef YNET_LOG_BUFFER
//     next(); //写入所有数据
// #endif
//     close(_openfd); //关闭文件描述符
// }






// #ifdef YNET_LOG_BUFFER
// //写入缓冲
// void Logger::Enqueue(std::string log)
// {
//     std::lock_guard<std::mutex> lock(_mutex);

//     int log_remain = log.size();    //日志剩余
//     int wd = 0;                     //已写
//     const char* logc = log.c_str(); 

//     while(log_remain != 0)
//     {
//         int worklen = strlen(workarray());          
//         int gap = ARRAY_SIZE-strlen(workarray());   //当前数组可写入
//         if(log_remain > gap)
//         {// 当前工作的buffer不够写入本条log
//             log_remain-=gap;
//             strncpy(workarray()+worklen,logc+wd,gap);
//             wd+=gap;
//             next();
//         }
//         else
//         {// 当前工作的buffer可以写入本条log
//             strncpy(workarray()+worklen,logc+wd,log_remain);
//             log_remain=0;
//         }
//     }
// }



// const char* Logger::GetFullArray()
// {
    
//     if(hasfulled())
//     {
//         const char* ret = _buffers[_pendingwriteindex].second;
//         _pendingwriteindex = _buffers[_pendingwriteindex].first;
//         return ret;
//     }
//     else
//         return nullptr;
// }


// void Logger::next()
// {
//     //是否需要扩张
//     if((_nowindex+1)%_nowsize == _pendingwriteindex)
//     {//扩张
//         int nextnext = _buffers[_nowindex].first;
//         _buffers.push_back(std::pair<int,char*>(nextnext,new char[ARRAY_SIZE]));
//         _buffers[_nowindex].first = _nowsize;
//         _nowsize++;
//         _nowindex = _buffers[_nowindex].first;
//     }
//     else//正常移动
//     {
//         _nowindex = _buffers[_nowindex].first;  //下一个节点
//         memset(workarray(),'\0',ARRAY_SIZE);
//     }
//     _cond.notify_all();
// }


// int Logger::resetbuffer(int index)
// {
//     int ret = -1;
//     if(index < _nowsize && index >= 0)
//     {
//         ret = 1;
//         memset(_buffers[index].second,'\0',ARRAY_SIZE);
//     }
//     return ret;
// }
// int Logger::flushbuffer(int index)
// {   
    
//     int n = strlen(_buffers[index].second);
//     int remain = 0;
//     //数据全部写入
//     while(n > 0)
//     {
//         int nbytes = write(_openfd,_buffers[index].second+remain,n);
//         remain += nbytes; n -= nbytes; 
//     }
//     memset(_buffers[index].second,'\0',ARRAY_SIZE);
//     //如果当前缓冲区是已满的待处理缓冲区，则移动待处理下标
//     if(index == _pendingwriteindex)
//         _pendingwriteindex = _buffers[_pendingwriteindex].first;    //向后顺延
    
//     return n==0;
// }



// void Logger::nextPending()
// {
// }
// #else

// void Logger::Enqueue(std::string log)
// {
//     std::lock_guard<std::mutex> lock(_mutex);
//     _queue.push(log);
// }

// bool Logger::Dequeue(std::string& str)
// {
//     std::lock_guard<std::mutex> lock(_mutex);
//     if(_queue.size()<=0)
//         return false;
//     str = _queue.front();   //取队首
//     _queue.pop();           //出队
//     return true;
// }

// #endif



// void Logger::Log(LOGLEVEL level ,const std::string str)
// {
//     if(LOG_LEVEL > level)
//         return;
//     //char log[128];
//     char log[1024];
//     memset(log,'\0',1024);

//     sprintf(log,"[%4d/%0.2d/%0.2d  %0.2d:%0.2d:%0.2d--%0.4d]",clock::year(),clock::month()+1,clock::day(),
//         clock::hour(),clock::minute(),clock::second(),clock::millisecond());

//     switch (level)
//     {
//     case LOGLEVEL::LOG_TRACE :
//         strcpy(log+strlen(log),LeveL[0]);
//         break;
//     case LOGLEVEL::LOG_DEBUG :
//         strcpy(log+strlen(log),LeveL[1]);
//         break;
//     case LOGLEVEL::LOG_INFO :
//         strcpy(log+strlen(log),LeveL[2]);
//         break;
//     case LOGLEVEL::LOG_WARN :
//         strcpy(log+strlen(log),LeveL[3]);
//         break;
//     case LOGLEVEL::LOG_ERROR :
//         strcpy(log+strlen(log),LeveL[4]);
//         break;
//     case LOGLEVEL::LOG_FATAL :
//         strcpy(log+strlen(log),LeveL[5]);
//         break;
//     default:
//         break;
//     }

//     strcpy(log+strlen(log),str.c_str());
//     strcpy(log+strlen(log),"\n");
//     Enqueue(log);

    
// #ifdef LOG_STDOUT 
//     std::string line;
//     if (level <= LOGLEVEL::LOG_INFO)
//         line = format("\033[0m\033[1;36m%s\033[0m",log);
//     else
//         line = format("\033[0m\033[1;31m%s\033[0m",log);
//     write(STDOUT_FILENO,line.c_str(),line.size());  //写
// #endif


// }



// std::string yrpc::util::logger::format(const char* fmt, ...)
// {
//     char        data[1024];
//     size_t      i = 0;
//     va_list     ap;
 
//     va_start(ap, fmt);
//     vsnprintf(data + i, sizeof(data) - i, fmt, ap);
//     va_end(ap);
 
//     return std::string(data);
// }