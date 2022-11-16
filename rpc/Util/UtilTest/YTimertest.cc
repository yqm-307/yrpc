#include "../TimeQueue.h"

using namespace yrpc::util;
using namespace yrpc::util::clock;


class timequeue
{
public:
    timequeue(clock::ms tick):tick_(tick){};
    void addtask(Timestamp<ms> ts,std::string& str)
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
    clock::ms tick_;
    YTimer<std::string*> queue_;
};


void test(timequeue& timer)
{
    std::string str[5];
    for(int i=0;i<5;++i)
    {
        str[i] = std::to_string(hour())+":"+std::to_string(minute()+i+1)+":"+std::to_string(second())+":"+std::to_string(millisecond())+ "\tid:"+std::to_string(i+1);
        timer.addtask(
            (now<ms>()+clock::s(i+1))
            ,str[i]);
    }
    timer.run();
}
int main()
{
    timequeue timer(10ms);
    printf("now %ld:%ld:%ld:%ld\n",hour(),minute(),second(),millisecond());
    test(timer);
}








