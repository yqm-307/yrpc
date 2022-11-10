#include "../RpcClient.h"
#include <memory>
#include "../../proto/test_protocol/AddAndStr.pb.h"
using namespace yrpc;
using namespace yrpc::util;
using namespace yrpc::util::clock;




typedef google::protobuf::Message   Message;
typedef std::shared_ptr<Message>    MessagePtr;

int main()
{   

    rpc::RpcClient client("127.0.0.1",12020);
    bool flag = true;

    while(1)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        
        if(client.IsConnected())
        {
            printf("连接成功!\n");
            auto req = std::make_shared<EchoReq>();
            req->set_str("hello world");
            auto call = rpc::CallObjFactory::GetInstance()->Create<EchoReq,EchoRsp>(req,[&](std::shared_ptr<google::protobuf::Message> rsp){
                std::shared_ptr<EchoRsp> result =  std::static_pointer_cast<EchoRsp>(rsp);
                printf("async call :  %s\n",result->str().c_str());
                flag = false;
            });
            client.Call(std::move(call));
            // client.Call();
            while(flag){
                std::this_thread::sleep_for(std::chrono::microseconds(200));
            }
            break;
        }
        else
            printf("尚未成功!\n");
    }

    std::shared_ptr<AddReq> req;
    MessagePtr p = req;

}