#include "../Epoller.h"
#include "../Hook.h"
using namespace yrpc::coroutine::poller;


Epoller scheduler(64*1024,65535);

struct Args
{
    Args(Epoller*poll)
    {
        cond.Init(poll,-1);
    }
    std::string str = "";
    yrpc::socket::Epoll_Cond_t cond;
};


void producer(void* args)
{
    auto arg = (Args*)args;
    for(int i=0;i<10;++i)
    {
        arg->str = "producer produce a task! "+std::to_string(yrpc::util::clock::minute()) +":"+std::to_string(yrpc::util::clock::second())+":"+std::to_string(yrpc::util::clock::millisecond());
        arg->cond.Notify();
        yrpc::socket::YRSleep(&scheduler,1000);
        printf("%s\n",arg->str.c_str());
    }
}

void consumer (void* args)
{
    auto arg = (Args*)args;
    for(int i=0;i<10;++i)
    {
        arg->str = "consumer consume a task! "+std::to_string(yrpc::util::clock::minute()) +":"+std::to_string(yrpc::util::clock::second())+":"+std::to_string(yrpc::util::clock::millisecond());
        arg->cond.Wait();   //等待信号
        printf("%s\n",arg->str.c_str());
    }
}


void test1()
{
    Args* arg = new Args(&scheduler);
    scheduler.AddTask(consumer,arg);
    scheduler.AddTask(producer,arg);

    scheduler.RunForever();
    scheduler.Loop();
}


//
void test2()
{
    scheduler.AddTask([](void*){
        yrpc::socket::Epoll_Cond_t cond;
        cond.Init(&scheduler, 1000);
        while (1)
        {
            printf("阻塞——");
            cond.Wait();
            printf("苏醒\n");
        }
    },nullptr);
   
    scheduler.RunForever();
    scheduler.Loop();
}


void test3()
{

    yrpc::socket::Epoll_Cond_t cond;
    cond.Init(&scheduler);
    scheduler.AddTask([](void*args){
        yrpc::socket::Epoll_Cond_t* cond = (yrpc::socket::Epoll_Cond_t*)(args);
        while (1)
        {
            printf("阻塞\n");
            cond->Wait();
            printf("苏醒\n");
        }

    },&cond);
    // scheduler.AddTask([](void*args){
    //     yrpc::socket::Epoll_Cond_t* cond = (yrpc::socket::Epoll_Cond_t*)(args);
    //     printf("唤醒\n");
    //     cond->Notify();
    // },&cond);
    cond.Notify();

    scheduler.RunForever();
    scheduler.Loop();
}

int main()
{
    //test2();
    test3();
}