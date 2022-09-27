#include "../Hook.h"

using namespace yrpc::socket;
using namespace yrpc::util::clock;

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


// sleep 实现让出
void test1( yrpc::coroutine::poller::Epoller* sche)
{
    int a[10];
    for(int i=0;i<10;++i)
    {
        a[i]=i;
        sche->AddTask([sche](void*arg){
            int id = *(int*)arg;
            while(1)
            {
                printf("协程ID : %d\n",id);
                YRSleep(sche,0);
            }
        },&a[i]);
    }
}

int main()
{
    yrpc::coroutine::poller::Epoller scheduler(64*1024,65535);
    printf("开始!\n");
    //scheduler.AddTask(test,&scheduler);
    
    test1(&scheduler);

    scheduler.RunForever();
    scheduler.Loop();
}