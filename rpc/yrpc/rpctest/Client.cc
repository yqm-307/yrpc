#include "../RpcClient.h"
#include <memory>
#include "../../proto/test_protocol/AddAndStr.pb.h"
using namespace yrpc;
using namespace yrpc::util;
using namespace yrpc::util::clock;

typedef google::protobuf::Message Message;
typedef std::shared_ptr<Message> MessagePtr;

std::atomic_int ccount{0};

void call_once(rpc::RpcClient& C,const std::string& once )
{
    auto pck = EchoReq();
    pck.set_str(once);
    assert(C.IsConnected());
    
    auto co = rpc::CallObjFactory::GetInstance()->Create<EchoReq,EchoRsp>(
        std::move(pck),"Echo",
        [](std::shared_ptr<google::protobuf::Message> rsp){
            ccount.fetch_add(1);
            printf("完成了第%d次\n",ccount.load());
        });
    assert(co);
    while (C.Call(co) < 0);
}

int main()
{
    yrpc::util::logger::Logger::GetInstance("clit.log");

    rpc::RpcClient client("127.0.0.1", 12020);
    // bool flag = true;

    while (1)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));

        if (client.IsConnected())
        {
            printf("连接成功!\n");

            break;
        }
    }

    for (int i = 0;i<10000;++i)
    {
        call_once(client,"hello world");
        // printf("注册一次\n");
    }

    while(ccount<10000){std::this_thread::sleep_for(std::chrono::milliseconds(10));}
    printf("注册over!\n");
}