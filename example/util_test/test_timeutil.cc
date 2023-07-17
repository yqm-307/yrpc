#include "yrpc/Util/Clock.h"
#include <unistd.h>
#include <bbt/timer/Clock.hpp>

using namespace yrpc::util::clock;
using namespace bbt::timer::clock;

//超时
void test1()
{
    auto p1 = now<ms>();
    sleep(1);

    if(expired<ms>(p1))
    {
        printf("超时了\n");
    }
    else
    {
        printf("出错\n");
    }
}


int main()
{
    test1();
}