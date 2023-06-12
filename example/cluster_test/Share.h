#pragma once
#include "../all_example.pb.h"
#include "yrpc/core/YRpc.h"
#include <memory>
#include <bbt/timer/interval.hpp>
#include <bbt/config/GlobalConfig.hpp>

using namespace yrpc;
using namespace yrpc::util;
using namespace yrpc::util::clock;

typedef google::protobuf::Message Message;
typedef std::shared_ptr<Message> MessagePtr;
typedef std::function<void(std::shared_ptr<google::protobuf::Message>)> SendPacket;



class TestService
{
public:
    TestService(){
        bbt::config::GlobalConfig::GetInstance()->GetDynamicCfg()->SetEntry(
                bbt::config::BBTSysCfg[bbt::config::BBT_LOG_STDOUT_OPEN], &debug_log);
        rpc::Rpc::register_service<AddReq,AddRsp>("add",std::bind(&TestService::AddHandle,this,std::placeholders::_1));
        rpc::Rpc::register_service<EchoReq,EchoRsp>("Echo",std::bind(&TestService::EchoHandle,this,std::placeholders::_1));
        
    }
    ~TestService(){}
    /* 非阻塞，启动监听和service */
    void Start()
    {
        rpc::Rpc::StartServerListen(yrpc::detail::net::YAddress(12020));
    }

    void Test_Random_Call()
    {

    }
private:

    MessagePtr AddHandle(MessagePtr args)
    {
        std::shared_ptr<AddReq> req = std::static_pointer_cast<AddReq>(args);
        int rval =  req->a();
        int lval = req->b();
        auto rsp = std::make_shared<AddRsp>();
        rsp->set_result(rval+lval);
        return rsp;
    }

    MessagePtr EchoHandle(const MessagePtr args)
    {
        const std::shared_ptr<EchoReq> req = std::static_pointer_cast<EchoReq>(args);
        std::string str = req->str();
        auto rsp = std::make_shared<EchoRsp>();
        rsp->set_str(str);
        return rsp;                       
    }
private:
    int debug_log{1};


};
