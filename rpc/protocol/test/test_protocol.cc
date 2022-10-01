#include "../YProtocolGenerater.h"
#include "../YProtocolResolver.h"


using namespace yrpc::detail::protocol;


// bool test1()
// {


//     RpcRequest heartbeat();
//     heartbeat.Message()->set_tick(1);
//     std::string byte;
//     heartbeat.ToByteArray(byte);
//     printf("type: %d\nid: %d\ntick: %d\nlen = %d\n",heartbeat.GetProtoType(),heartbeat.GetProtoID(),heartbeat.Message()->tick(),byte.size());


//     RpcResponse heartbeat_rsp(byte);
//     RpcResponse(S2C_HEARTBEAT_RSP)::ProtoType bp;
//     heartbeat_rsp.ToProtoMsg(bp);
//     printf("type: %d\nid: %d\ntick: %d\n",heartbeat_rsp.GetProtoType(),heartbeat_rsp.GetProtoID(),bp.tick());

//     return true;
// }

int main(int argc,char* argv[])
{
    // if(!test1())
    //     printf("protocol test1 fatal!\n");
}   