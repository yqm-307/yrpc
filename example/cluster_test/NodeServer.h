#pragma once
#include <yrpc/core/YRpc.h>
#include "../all_example.pb.h"

class NodeServer
{
public:
    NodeServer(yrpc::rpc::Address, int port);
    ~NodeServer(){}
private:
    void Register();
    /* 此节点提供的remote call */
    yrpc::rpc::MessagePtr Add(yrpc::rpc::MessagePtr pck);
    yrpc::rpc::MessagePtr Echo(yrpc::rpc::MessagePtr pck);

    /*  */
    yrpc::rpc::MessagePtr Insert(yrpc::rpc::MessagePtr pck);
    yrpc::rpc::MessagePtr Delete(yrpc::rpc::MessagePtr pck);
    yrpc::rpc::MessagePtr Search(yrpc::rpc::MessagePtr pck);

    yrpc::rpc::MessagePtr HeartBeat(yrpc::rpc::MessagePtr pck);


private:
    std::vector<int>    m_array;

};