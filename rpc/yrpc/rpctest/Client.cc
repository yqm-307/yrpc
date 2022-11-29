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
    
    auto co = rpc::CallObjFactory::GetInstance()->Create<EchoReq,EchoRsp>(
        pck,"Echo",
        [](std::shared_ptr<google::protobuf::Message> rsp){
            ccount.fetch_add(1);
        });
    
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
    }
    printf("over!\n");
}