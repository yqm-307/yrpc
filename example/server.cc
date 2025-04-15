#include <bbt/rpc/RpcServer.hpp>

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

    if (auto err = server->Init("", 10031, 3000); err.has_value())
    {
        std::cout << "Init failed: " << err.value().What() << std::endl;
        return -1;
    }

    if (auto err = server->RegisterMethod("test_method", []
        (std::shared_ptr<bbt::rpc::RpcServer> server, bbt::network::ConnId connid, bbt::rpc::RemoteCallSeq seq, const bbt::core::Buffer& data) {
        bbt::rpc::detail::RpcCodec codec;
        auto [err, values] = codec.Deserialize(data);

        if (err.has_value())
        {
            std::cout << "Deserialize failed: " << err.value().What() << std::endl;
            return;
        }
        std::tuple<int32_t, int64_t, int32_t, std::string> tuple;
        if (err = codec.DeserializeWithArgs(data, tuple); err.has_value())
            std::cerr << "codec.DeserializeWithArgs bad! " << err->CWhat() << std::endl;

        std::cout << "Tuple contents: "
              << std::get<0>(tuple) << ", "
              << std::get<1>(tuple) << ", "
              << std::get<2>(tuple) << ", "
              << std::get<3>(tuple) << std::endl;
        server->DoReply(connid, seq, "nothing happened!");
    }); err.has_value())
    {
        std::cout << "RegisterMethod failed: " << err.value().What() << std::endl;
        return -1;
    }

    io_thread->Start();
    io_thread->Join();
}