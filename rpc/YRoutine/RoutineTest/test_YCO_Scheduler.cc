#include "../Scheduler.h"
#include <stdio.h>
#include <time.h>

using namespace yrpc::coroutine::detail;

typedef struct tagTestArgs {
    YCO_Scheduler * scheduler;
    int id;
    int seq;
} TestArgs_t;






//调度器，计数
void execute(YCO_Scheduler & runtime, size_t count) {
    srandom (time(NULL) );
    TestArgs_t * args = new TestArgs_t[count]; //新建一个协程调度器
    printf("%x\n",args);
    //创建协程
    for( size_t i = 0; i < count; i++ ) {
        args[i].seq = i;
        args[i].scheduler = &runtime;
        args[i].id = runtime.Add([](void* args)
        {
            TestArgs_t * tt = (TestArgs_t*) args;
            printf("%d START\n", tt->id);
            for (int i = 0; i < 8; i++) {
                printf("%d : %d\n", tt->id, i);
                tt->scheduler->Yield();
            }
            printf("%d END\n", tt->id);
        }
        , &(args[i]) );}
    
    int n = 0;
    //随机唤醒协程
    while( ! runtime.Empty() ) 
    {
        int seq = rand() % count;
        //int seq = n++%count;
        runtime.Resume( args[ seq ].id );
    }

    //双重释放 第一次new 和 第二次new 数组地址相同？
    printf("准备free : %x\n",args);
    delete []args;
    printf("free完毕");
}




void run(size_t count) {
    YCO_Scheduler runtime(64 * 1024, false);   //协程栈 64kb，不加锁
    execute(runtime, count);
    printf("once \n");

    //
    execute(runtime, count);
}




int main(int argc, char * argv[]) {

    run(2);
    printf("main exit\n");
    return 0;
}

