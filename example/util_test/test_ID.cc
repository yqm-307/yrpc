#include "../IDGenerate.h"
#include <map>
#include <set>
#include <thread>
using namespace yrpc::util::id;

/*  */
void test1()
{

    printf("32位每毫秒生成10w\n");

    for (int i = 0; i < 10; ++i)
    {
        printf("一轮10w次生成\n");
        std::set<uint32_t> idmap;
        for (int j = 0; j < 100000; ++j)
        {
            if (!idmap.insert(::GenerateID::GetIDuint32()).second)
                std::cout << "重复" << std::endl;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}


/**
 *  测试结果非常不理想，冲突概率非常高 
 */
void test2()
{
    printf("32位每毫秒生成10w\n");

    for (int i = 0; i < 10; ++i)
    {
        printf("一轮10w次生成\n");
        std::set<uint32_t> idmap;
        for (int j = 0; j < 100000; ++j)
        {
            if (!idmap.insert(::GenerateID::GetIDuint32_unsafe()).second)
                std::cout << "重复" << std::endl;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        // std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}

int main()
{
    test1();
    // test2(); // 需要改进
}