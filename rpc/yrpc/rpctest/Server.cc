#include "../RpcServer.h"
#include "../../proto/test_protocol/AddAndStr.pb.h"
// #include "AddAndStr.pb.h"
#include <memory>
using namespace yrpc;
using namespace yrpc::util;
typedef std::function<void(std::shared_ptr<google::protobuf::Message>)> SendPacket;
typedef std::shared_ptr<google::protobuf::Message> MessagePtr;



void AddHandle(MessagePtr args,SendPacket&& onsend)
{
    std::shared_ptr<AddReq> req = std::static_pointer_cast<AddReq>(args);
    int rval =  req->a();
    int lval = req->b();
    auto rsp = std::make_shared<AddRsp>();
    rsp->set_result(rval+lval);
    onsend(rsp);
}

void EchoHandle(const MessagePtr args,SendPacket&& onsend)
{
    const std::shared_ptr<EchoReq> req = std::static_pointer_cast<EchoReq>(args);
    std::string str = req->str();
    auto rsp = std::make_shared<EchoRsp>();
    rsp->set_str(str);
    onsend(rsp);
}

int main()
{
    yrpc::util::logger::Logger::GetInstance("sevr.log");
    auto sev = yrpc::rpc::RpcServer::GetInstance();
    sev->SetAddress(yrpc::detail::net::YAddress(12020));
    sev->SetThreadPool(0);

    sev->register_service<AddReq,AddRsp>("add",AddHandle);
    sev->register_service<EchoReq,EchoRsp>("Echo",EchoHandle);
    sev->Start();
}

