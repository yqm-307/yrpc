#include "NodeServer.h"

NodeServer::NodeServer(yrpc::rpc::Address, int port)
{
    // yrpc::rpc::Rpc::register_service<ServiceRegistReq, ServiceRegistRsp>([](){

    // });
}

// void Register()
// {
//     yrpc::rpc::Rpc::register_service<AddReq, AddRsp>("Add", 
//     [](){yrpc::rpc::MessagePtr msg,yrpc::rpc::detail::RpcSession::SPtr sess}{

//     }
//     );
// }

// yrpc::rpc::MessagePtr NodeServer::Add(yrpc::rpc::MessagePtr pck)
// {

// }
// yrpc::rpc::MessagePtr NodeServer::Echo(yrpc::rpc::MessagePtr pck);

// /*  */
// yrpc::rpc::MessagePtr NodeServer::Insert(yrpc::rpc::MessagePtr pck);
// yrpc::rpc::MessagePtr NodeServer::Delete(yrpc::rpc::MessagePtr pck);
// yrpc::rpc::MessagePtr NodeServer::Search(yrpc::rpc::MessagePtr pck);