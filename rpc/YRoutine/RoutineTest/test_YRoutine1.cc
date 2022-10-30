#include "../YRoutineContext.h"
using namespace yrpc;


// 声明函数 f1
void f1(void *);

// 协程上下文
yrpc::coroutine::context::YRoutineContext c1(64 * 1024, &f1, nullptr, nullptr, true);


void f1(void *) {
    printf("f1 第一次进入\n");
    printf("f1 第一次Yield\n");
    c1.Yield();
    printf("f1 第二次进入\nf1运行结束\n");
    c1.Yield();
    printf("not never back\n");
}



int main(int argc, char ** argv) 
{
    
    printf("进入主函数\t唤醒f1\n");
    c1.Resume();

    printf("第二次进入主函数\t唤醒f1\n");
    c1.Resume();
    printf("main 退出");
    return 0;
}