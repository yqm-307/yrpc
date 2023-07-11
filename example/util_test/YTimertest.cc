#include "yrpc/Util/TimeQueue.h"

using namespace yrpc::util;
using namespace yrpc::util::clock;


class timequeue
{
public:
    timequeue(bbt::timer::clock::ms tick):tick_(tick){};
    void addtask(bbt::timer::clock::Timestamp<bbt::timer::clock::ms> ts,std::string& str)
    {
        queue_.AddTask(ts,&str);
    }
    void run()
    {
        while(queue_.ThreadSleepFor(tick_))
        {
            std::vector<YTimer<std::string*>::Ptr> strs;
            queue_.GetAllTimeoutTask(strs);
            for(auto p : strs)
                printf("触发事件: %s\n",p->Data()->c_str());            
        }
    }

private:
    bbt::timer::clock::ms tick_;
    YTimer<std::string*> queue_;
};


void test(timequeue& timer)
{
    std::string str[5];
    for(int i=0;i<5;++i)
    {
        str[i] = std::to_string(bbt::timer::clock::hour())+":"+std::to_string(bbt::timer::clock::minute()+i+1)+":"+std::to_string(bbt::timer::clock::second())+":"+std::to_string(bbt::timer::clock::millisecond())+ "\tid:"+std::to_string(i+1);
        timer.addtask(
            (bbt::timer::clock::now<bbt::timer::clock::ms>() + bbt::timer::clock::s(i+1))
            ,str[i]);
    }
    timer.run();
}
int main()
{
    timequeue timer(bbt::timer::clock::ms(10));
    printf("now %ld:%ld:%ld:%ld\n",bbt::timer::clock::hour(),bbt::timer::clock::minute(),bbt::timer::clock::second(),bbt::timer::clock::millisecond());
    test(timer);
}








