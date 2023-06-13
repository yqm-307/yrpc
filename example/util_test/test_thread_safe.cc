#include "../cter_tsf.h"
#include <thread>
#include <bbt/random/random.hpp>
using namespace yrpc::util::thread_safe_cter;

void nthread_map()
{
    thread_saft_cter<int,int,std::map<int,int>> map;
    bbt::random::mt_random<int,INT32_MIN,INT32_MAX> rd;
    thread_saft_cter_o<int,std::queue<int>> queue;
    std::thread* t[50];
    for(int i=0;i<50;++i)
    {

        t[i] = new std::thread([&map, &rd, &queue]()
                               {
        while(1)
        {
            int rr = rd();
            map.Insert(std::make_pair<int,int>(std::move(rr),2));
            queue.Push(rr);
        } });
        t[i]->detach();
    }
    while (1)
    {
        if (!queue.Empty())
            map.Remove(queue.Pop());
    }
}


void testqueue()
{
    thread_saft_cter_o<int,std::queue<int>> queue;
    
    std::thread t([&queue](){

        while(1)
        {
            for (int i = 0; i < 100000; ++i)
            {
                if (!queue.Empty())
                {
                    queue.Pop();
                }
            }
            printf("10w pop\n");
        }
    });

    for(int i=0;i<1000000;i++)
    {
        queue.Push(i);
        if(i%100000 == 0)
            printf("10w push\n");
    }
    t.join();
}

int main()
{

    nthread_map();

}
