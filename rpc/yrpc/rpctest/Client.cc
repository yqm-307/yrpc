#include "../RpcClient.h"
#include "../../proto/AddAndStr.pb.h"
#include <bbt/timer/interval.hpp>
using namespace yrpc;
using namespace yrpc::util;
using namespace yrpc::util::clock;
rpc::RpcClient client("127.0.0.1",12020,"RpcClient.log");



// //同步调用
// void test4(std::string str)
// {
//     auto req = std::make_shared<EchoReq>();
//     req->set_str(str);


//     bbt::timer::interval begin;
    
//     std::shared_ptr<EchoRsp> buf;

//     for (int i = 0; i < 1000; ++i)
//     {
//         bbt::timer::interval call_once;
//         auto ptr = client.result_long<EchoReq, EchoRsp>("Echo", req);
//         ptr->Get(buf);
//         printf(" %d :\n",i);
//         printf("调用耗时: %d\n",call_once.intervalnow<ms>());
//     }

//     printf("1000 同步调用结束 ,耗时 : %d ms\n",begin.intervalnow<yrpc::util::clock::ms>());
// }

// //异步调用
// void test5(std::string str,rpc::RpcClient* client)
// {
// auto req = std::make_shared<EchoReq>();
//     req->set_str(str);
//     EchoRsp result;

//     bbt::timer::interval begin;
    
//     EchoRsp buf;
//     int times=0;
//     for (int i = 0; i < 100000; ++i)
//     {
//         client->async_call_long<EchoReq,EchoRsp>("Echo",req,[&times](EchoRsp& ptr){
//             ++times;
//         }); 
//     }
//     int mms=0;
//     while(times < 100000)
//     {
//     }
//     printf("100000 异步调用结束 ,耗时 : %d ms\n",begin.intervalnow<yrpc::util::clock::ms>());

// }

// void nthread_test(std::string str)
// {
//     yrpc::util::lock::CountDownLatch lock(10);
//     for (int i = 0; i < 2;++i)
//     {
//         new std::thread([&str,&lock]()
//         {
//             rpc::RpcClient* client = new rpc::RpcClient("127.0.0.1",12020);
//             test5(str,client);
//             lock.down();
//         });
//         printf("开启线程\n");
//     }
//     lock.wait();
//     sleep(1);
    
// }


void new_async_call()
{
    std::string str= "hello world"; int once = 100000;
    std::shared_ptr<EchoReq> req = std::make_shared<EchoReq>();
    req->set_str(str);
    std::atomic_int i =0;
    bbt::timer::interval begin;
    int t=0;
    while (t<once)
    {

        bool is = client.async_call("Echo", req, [&i,&str](std::shared_ptr<google::protobuf::Message> msg)
                          {
        std::shared_ptr<EchoRsp> sptr = std::dynamic_pointer_cast<EchoRsp>(msg);
        if(sptr->str() == str)
        {
            ++i;
        } });
        if(is == true)
            once++;
    }
    while(i < once){}
    printf("%d次异步调用结束 ,耗时 : %d ms\n",once,begin.intervalnow<yrpc::util::clock::ms>());
}


int main()
{
    
    int args[2]{1,2};
    char str[1024]="";
    for(int i=0;i<100;++i)
    {
        str[i]='a';
    }


    // test5(str,&client);    //async_long 异步非阻塞
    new_async_call();
}