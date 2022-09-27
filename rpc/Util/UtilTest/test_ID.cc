#include "../IDGenerate.h"
#include <thread>
using namespace yrpc::util::id;

/*  */
void test1()
{

    printf("32位每毫秒生成10w\n");

    for (int i = 0; i < 1000; ++i)
    {
        for (int j = 0; j < 100000; ++j)
        {
            if (!::GenerateID::GetIDuint32())
                std::cout << "重复" << std::endl;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}

void test2()
{
    printf("32位每毫秒生成10w\n");

    for (int i = 0; i < 1000; ++i)
    {
        for (int j = 0; j < 100000; ++j)
        {
            if (!::GenerateID::GetIDuint32_unsafe())
                std::cout << "重复" << std::endl;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}

int main()
{
    test1();
    test2();
}