#include <bbt/rpc/RpcServer.hpp>
#include "proto.hpp"

using namespace bbt::rpc;

class MyServer : public bbt::rpc::RpcServer
{
public:

    MyServer(std::shared_ptr<bbt::network::EvThread> io_thread):
        bbt::rpc::RpcServer(io_thread)
    {}

    void OnError(const bbt::core::errcode::Errcode& err) override
    {
        std::cout << "OnError: " << err.What() << std::endl;
    }

    void OnTimeout(bbt::network::ConnId) override
    {
        std::cout << "OnTimeout: " << std::endl;
    }
};

int main()
{
    auto evloop = std::make_shared<bbt::pollevent::EventLoop>();
    auto io_thread = std::make_shared<bbt::network::EvThread>(evloop);

    auto server = std::make_shared<MyServer>(io_thread);

    if (auto err = server->Init("", 10031, 10000); err.has_value())
    {
        std::cout << "Init failed: " << err.value().What() << std::endl;
        return -1;
    }

    if (auto err = server->RegisterMethod("test_method", []
        (std::shared_ptr<bbt::rpc::RpcServer> server, bbt::network::ConnId connid, bbt::rpc::RemoteCallSeq seq, const bbt::core::Buffer& data) ->bbt::core::errcode::ErrOpt {
        Test1Request req;
        if (auto err = codec::DeserializeWithTuple(data, req); err.has_value())
            return err;

        std::cout << "Tuple contents: "
              << std::get<0>(req) << ", "
              << std::get<1>(req) << ", "
              << std::get<2>(req) << ", "
              << std::get<3>(req) << std::endl;
        
        Test1Reply results{bbt::rpc::emRpcReplyType::RPC_REPLY_TYPE_SUCCESS, "nothing happened!"};
        server->DoReply(connid, seq, results);

        return std::nullopt;
    }); err.has_value())
    {
        std::cout << "RegisterMethod failed: " << err.value().What() << std::endl;
        return -1;
    }

    io_thread->Start();
    io_thread->Join();
}