#include "../SessionBuffer.h"
#include "../../protocol/YProtocolGenerater.h"
#include "../../protocol/YProtocolResolver.h"
#include "../../proto/yrpc_core_protocol/c2s.pb.h"
#include "../../proto/test_protocol/AddAndStr.pb.h"
#include <bbt/timer/interval.hpp>
using namespace std;
using namespace yrpc;
using namespace yrpc::detail::protocol;



// protocol 解析和编码
void test_1()
{
    yrpc::detail::net::SessionBuffer packetmgr;
    const int times = 100000;
    srand((uint32_t)time(NULL));

    uint32_t bk[2] = {yrpc::util::hash::BKDRHash("add"),yrpc::util::hash::BKDRHash("heart")};

    bbt::timer::interval time;
    for(int i =0 ;i<times;++i)
    {
        if (rand()%2 == 0)
        {

            auto pck = std::make_shared<C2S_HEARTBEAT_REQ>();
            pck->set_tick(100000);

            YProtocolGenerater p(pck,bk[1],detail::protocol::define::type_C2S_RPC_CALL_REQ);
            std::string bytes;
            p.ToByteArray(bytes);
            packetmgr.Append(bytes.c_str(),bytes.size());
        }
        else
        {
            auto pck = std::make_shared<AddRsp>();
            pck->set_result(200000);
            YProtocolGenerater p(pck,bk[0],detail::protocol::define::type_S2C_RPC_CALL_RSP);
            std::string bytes;
            p.ToByteArray(bytes);
            packetmgr.Append(bytes.c_str(),bytes.size());
        }
    }


    for(int i = 0;i<times;++i)
    {
        std::string tmp;
        tmp = packetmgr.GetAPck();
        YProtocolResolver res(tmp);
        // res.ToProtoMsg(ptr);
        // if (res.GetServiceID() == bk[0])
        // {
        //     auto ptr = std::make_shared<AddRsp>();
        //     if (res.ToProtoMsg(ptr))
        //         printf("result = %d\n",ptr->result());
        // }
        // else
        // {
        //     auto ptr = std::make_shared<C2S_HEARTBEAT_REQ>();
        //     if (res.ToProtoMsg(ptr))
        //         printf("result = %d\n",ptr->tick());
            
        // }
        // if(tmp.size() > 0)
        // {
        //    printf("succ %d  size=%d\n",i,tmp.size()); 
        // }
    }

    printf("%d 次En/Decode %ld ms\n",times,time.intervalnow());
}

// 正常序列化反序列化流程
void test_2()
{
    auto ptr = std::make_shared<C2S_HEARTBEAT_REQ>();
    ptr->set_tick(110001);
    YProtocolGenerater gen(ptr,yrpc::util::hash::BKDRHash("tmp"),detail::protocol::define::type_C2S_RPC_CALL_REQ);

    std::string bytes;
    gen.ToByteArray(bytes);

    YProtocolResolver rlv(bytes);
    auto result = std::make_shared<C2S_HEARTBEAT_REQ>();

    if ( rlv.ToProtoMsg(result) )
    {
        printf("ok!\n");
    }


}


int main()
{
    test_1();
    // test_2();
}