#include <yrpc/RpcServer.hpp>

class MyServer : public yrpc::RpcServer
{
public:

    MyServer(std::shared_ptr<bbt::network::EvThread> io_thread):
        yrpc::RpcServer(io_thread)
    {}

    void OnError(const bbt::core::errcode::Errcode& err) override
    {
        std::cout << "OnError: " << err.What() << std::endl;
    }
};

int main()
{
    auto evloop = std::make_shared<bbt::pollevent::EventLoop>();
    auto io_thread = std::make_shared<bbt::network::EvThread>(evloop);

    auto server = std::make_shared<MyServer>(io_thread);

    if (auto err = server->Init("", 10031, 10000, 3000); err.has_value())
    {
        std::cout << "Init failed: " << err.value().What() << std::endl;
        return -1;
    }

    if (auto err = server->RegisterMethod("test_method", []
        (std::shared_ptr<yrpc::RpcServer> server, bbt::network::ConnId connid, yrpc::RemoteCallSeq seq, const bbt::core::Buffer& data) {
        yrpc::detail::RpcCodec codec;
        auto [err, values] = codec.Deserialize(data);

        if (err.has_value())
        {
            std::cout << "Deserialize failed: " << err.value().What() << std::endl;
            return;
        }
        if (values.size() != 4)
        {
            std::cout << "Invalid number of arguments" << std::endl;
            return;
        }
        for (auto& value : values)
        {
            std::cout << "Field Type: " << (int)value.header.field_type << ", Length: " << value.header.field_len << value.value.int64_value << value.string << std::endl;
        }

        server->DoReply(connid, seq, "nothing happened!");
    }); err.has_value())
    {
        std::cout << "RegisterMethod failed: " << err.value().What() << std::endl;
        return -1;
    }

    io_thread->Start();
    io_thread->Join();
}