#pragma once
#include <bbt/network/TcpServer.hpp>
#include <bbt/network/TcpClient.hpp>
#include <bbt/network/EvThread.hpp>

namespace yrpc
{

class RpcServer;
class RpcClient;

typedef std::function<void(const bbt::core::Buffer&)> RpcMethod;

enum emErr
{
    ERR_COMM = 0,
    ERR_METHOD_ALREADY_REGISTERED
};

}