#include "../Hook.h"

using namespace yrpc::socket;
using namespace yrpc::util::clock;

int timeout_ms=500;

void test(void*args)
{
    auto loop = (yrpc::coroutine::poller::Epoller*)args;
    
    for(int i=0;i<10;++i)
    {
        printf("开始休眠 : %d:%d:%d\n",minute(),second(),millisecond());
        YRSleep(loop,1000);
        printf("休眠结束 : %d:%d:%d\n",minute(),second(),millisecond());
    }

}


// 普通task 的Sleep
void test1( yrpc::coroutine::poller::Epoller* sche)
{
    int* a= (int*)malloc(40);
    for(int i=0;i<10;++i)
    {
        a[i]=i;
        sche->AddTask([sche](void*arg){
            int id = *(int*)arg;
            for(int i=0;i<10;++i)
            {
                printf("协程ID : %d\n",id);
                YRSleep(sche,timeout_ms);
            }
            printf("Routinue:%d  exit\n",id);
        },(a+i));
    }
}



// 普通 task 的Reset timer
void test2( yrpc::coroutine::poller::Epoller* sche)
{   
    // 触发10次
    sche->AddTimer([](){
        printf("%ld\n",yrpc::util::clock::now<yrpc::util::clock::ms>().time_since_epoch().count());
    },1000,1000,10);
}

void test3( yrpc::coroutine::poller::Epoller* sche)
{
    auto* value = new std::atomic_int64_t(0);
    for (int i=0;i<50;++i)
    {
        std::thread th([sche,value](){
            while(1)
            sche->AddTimer([value](){
                (*value)++;
            },50);
        });
        th.detach();
    }
    sche->AddTimer([value](){
        printf("once : %d\n",value->load());
    },1000,1000,-1);
    printf("启动50个线程\n");
}



int main(int argc,char* args[])
{
    // if (argc < 2)
    // {
    //     perror("usage: [timeout ms]\n");
    //     exit(-1);
    // }
    // char* tmp;
    // timeout_ms = strtol(args[1],&tmp,10);
    // if (timeout_ms == 0)
    // {
    //     perror("args error!\n");
    //     exit(-2);
    // }


    yrpc::coroutine::poller::Epoller scheduler(64*1024,65535);
    printf("开始!\n");
    //scheduler.AddTask(test,&scheduler);
    
    // test1(&scheduler);
    // test2(&scheduler);
    test3(&scheduler);

    scheduler.RunForever();
    scheduler.Loop();
}