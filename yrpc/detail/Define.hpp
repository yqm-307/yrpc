#pragma once
#include <boost/noncopyable.hpp>

#include <bbt/network/TcpServer.hpp>
#include <bbt/network/TcpClient.hpp>
#include <bbt/network/EvThread.hpp>

namespace yrpc
{

class RpcServer;
class RpcClient;

typedef bbt::network::ConnId ConnId;
typedef uint64_t RpcMethodHash;
typedef uint64_t RemoteCallSeq;

typedef std::function<void(std::shared_ptr<RpcServer> server, ConnId connid, RemoteCallSeq seq, const bbt::core::Buffer& data)> RpcMethod;
typedef std::function<void(bbt::core::errcode::ErrOpt, const bbt::core::Buffer&)> RpcReplyCallback;

enum emErr
{
    ERR_COMM = 0,
    ERR_METHOD_ALREADY_REGISTERED,
    ERR_BAD_PROTOCOL,
    ERR_CLIENT_CLOSE,
    ERR_SERVER_NO_METHOD,
};

}