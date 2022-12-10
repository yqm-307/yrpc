#include "../Buffers.h"

typedef yrpc::util::buffer::Buffer Buffer;


// 写入测试
void test1()
{
    Buffer buf;
    srand((unsigned)time(NULL));
    for(int i=0;i<100;++i)
    {
        int n = 500+rand()%500;
        char* tmp = (char*)malloc(n);
        memset(tmp,'a',n);
        buf.WriteString(tmp,n);
    }

}


void test2()
{
    
}

int main()
{
    test1();
}