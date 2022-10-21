#include "../logger.h"
#include "../TimeQueue.h"
#include <bbt/random/random.hpp>
#include <unistd.h>
using namespace yrpc::util::logger;
using namespace yrpc::util;

void nthread()
{
    std::vector<std::thread*> ths;
    for(int i=0;i<10;++i)
    {
        
        ths.push_back(new std::thread([i](){
            bbt::random::mt_random<int,50,100> rand;
            while(1)
            {
                INFO("this is a log! %d",i);
                std::this_thread::sleep_for(yrpc::util::clock::ms(rand()));
            }
        }));
    }
}


int main()
{
    nthread();
    sleep(INT32_MAX);
}
