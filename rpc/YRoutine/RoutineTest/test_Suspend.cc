#include "../Hook.h"
#include <stdio.h>

using namespace yrpc::coroutine::poller;


void test1(Epoller* sche)
{
    int a[10];
    for(int i=0;i<10;++i)
    {
        a[i]=i;
        sche->AddTask([sche](void*arg){
            while(1)
            {
                printf("Routine %d\n",*(int*)arg);
                sche->Yield();
            }
        },&a[i]);
    }
}

int main()
{   
    Epoller* p = new Epoller(64*1024,65535);
    test1(p);
    p->RunForever();
    p->Loop();
}