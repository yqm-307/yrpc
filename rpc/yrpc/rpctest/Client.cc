#include "../RpcClient.h"
#include "../../proto/test_protocol/AddAndStr.pb.h"
#include <memory>
#include <bbt/timer/interval.hpp>
#include <bbt/config/GlobalConfig.hpp>
using namespace yrpc;
using namespace yrpc::util;
using namespace yrpc::util::clock;

typedef google::protobuf::Message Message;
typedef std::shared_ptr<Message> MessagePtr;

std::atomic_int ccount{0};
// int ccount=0;

void call_once(rpc::RpcClient& C,const std::string& once )
{
    auto pck = EchoReq();
    pck.set_str(once);
    assert(C.IsConnected());
    
    auto co = rpc::CallObjFactory::GetInstance()->Create<EchoReq,EchoRsp>(
        std::move(pck),"Echo",
        [](std::shared_ptr<google::protobuf::Message> rsp){
            // ccount.fetch_add(1,std::memory_order_seq_cst);
            ++ccount;
            // printf("完成了第%d次\n",ccount.load());
        });
    assert(co);
    while (C.Call(co) < 0);
}

void run_for_times(const std::string& ip,int port,int ntimes=-1)
{
    rpc::RpcClient client("127.0.0.1", 12020);
    yrpc::util::lock::Sem_t cond;
    ntimes = ntimes > 0 ? ntimes : 100000;
    bbt::timer::interval now;
    client.AsyncConnect([&](){

        for (int i = 0;i<ntimes;++i)
        {
            call_once(client,"hello world");
        }
    });
    while(ccount<ntimes){std::this_thread::sleep_for(std::chrono::milliseconds(5));}
    printf("调用 over!\n");
    printf("耗时: %ldms\n",now.intervalnow());
}

int main(int nparam,char* argv[])
{
    // if ( nparam != 3 )
    // {
    //     printf("usage : [ip] [port]");
    //     exit(-1);
    // }

    int flag = 1;
    bbt::config::GlobalConfig::GetInstance()->GetDynamicCfg()->SetEntry(bbt::config::BBTSysCfg[bbt::config::BBT_LOG_STDOUT_OPEN], &flag);
    // std::string ip(argv[1],sizeof(argv[1]));
    // int port = std::stoi(std::string(argv[2],sizeof(argv[2])));
    
    int port = 12020;
    std::string ip("127.0.0.1");

    run_for_times(ip,port,-1);
}