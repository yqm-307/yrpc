/**
 *  服务注册和服务匹配 单元测试 
 */


#include "../RpcServer.h"
#include "testproto.pb.h"   //proto生成 add 的请求类(AddReq)和响应类(AddRsp)

int main()
{
    yrpc::rpc::RpcServer server(114514,0);   //端口 114514 ; 4线程

    server.register_service<AddReq, AddRsp>("add", [](std::any args) -> google::protobuf::Message*
    {
        auto ptr = std::any_cast<std::shared_ptr<AddRsp>>(args);
        AddReq* req = (AddReq*)(ptr.get());
        int rval =  req->first();
        int lval = req->second();
        auto rsp = new AddRsp();
        rsp->set_result(rval+lval);
        return rsp; 
    });


    server.start(); //会自动开启另外3个线程以及协程调度器
}

