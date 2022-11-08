#include "../RpcClient.h"
#include "../../proto/test_protocol/AddAndStr.pb.h"
using namespace yrpc;
using namespace yrpc::util;
using namespace yrpc::util::clock;






int main()
{   

    rpc::RpcClient client("127.0.0.1",12020);
    
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));   // 休眠1s
    while(1)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        if(client.IsConnected())
        {
            printf("连接成功!\n");
            break;
        }
        else
            printf("尚未成功!\n");

    }

}