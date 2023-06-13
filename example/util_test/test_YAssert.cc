#include "yrpc/Util/Assert.h"


void test1()
{
    YAssert(true,"%d",1);

    YAssert(false,"%s","failed");
}


int main()
{

    test1();
}
