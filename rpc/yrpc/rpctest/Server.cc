#include "../RpcServer.h"
#include "../../proto/test_protocol/AddAndStr.pb.h"
// #include "AddAndStr.pb.h"
using namespace yrpc;
using namespace yrpc::util;


google::protobuf::Message* AddHandle(std::any args)
{
    auto ptr = std::any_cast<std::shared_ptr<AddReq>>(args);
    AddReq* req = (AddReq*)(ptr.get());
    int rval =  req->a();
    int lval = req->b();
    auto rsp = new AddRsp();
    rsp->set_result(rval+lval);
    return rsp;
}

google::protobuf::Message* EchoHandle(std::any args)
{
    auto ptr = std::any_cast<std::shared_ptr<EchoReq>>(args);
    EchoReq * req = (EchoReq*)(ptr.get());
    std::string str = req->str();
    auto rsp = new EchoRsp();
    rsp->set_str(str);
    return rsp;
}

int main()
{
    auto sev = yrpc::rpc::RpcServer::GetInstance();
    sev->SetAddress(yrpc::detail::net::YAddress(12020));
    sev->SetThreadPool(2);

    sev->register_service<AddReq,AddRsp>("add",AddHandle);
    sev->register_service<EchoReq,EchoRsp>("Echo",EchoHandle);
    sev->Start();
}

