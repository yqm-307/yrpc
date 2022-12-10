/**
 *  服务注册和服务匹配 单元测试 
 */


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

    // 问题可以复现

    
    rpc::RpcClient client("127.0.0.1",12020);
    // auto req = std::make_shared<EchoReq>();
    
    auto req = EchoReq();
    req.set_str("hello world");

    auto call = rpc::CallObjFactory::GetInstance()->Create<EchoReq,EchoRsp>(std::move(req),"Echo",
        [](std::shared_ptr<google::protobuf::Message> rsp){
    });
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    
    client.Call(call);


}

