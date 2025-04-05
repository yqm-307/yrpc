#include <yrpc/RpcClient.hpp>

class MyClient : public yrpc::RpcClient
{
public:
    MyClient(std::shared_ptr<bbt::network::EvThread> io_thread):
        yrpc::RpcClient(io_thread)
    {}

    void OnError(const bbt::core::errcode::Errcode& err)
    {
        std::cout << "OnError: " << err.What() << std::endl;
    }
private:
};

int main()
{
    auto evloop = std::make_shared<bbt::pollevent::EventLoop>();
    auto io_thread = std::make_shared<bbt::network::EvThread>(evloop);

    auto client = std::make_shared<MyClient>(io_thread);

    client->Init("127.0.0.1", 10031, 10000, 0, [](std::shared_ptr<yrpc::RpcClient> client) {
        std::cout << "Connected to server!" << std::endl;

        auto callback = [](bbt::core::errcode::ErrOpt err, const bbt::core::Buffer& buffer) {
            yrpc::detail::RpcCodec codec;
            auto [errdecode, values] = codec.Deserialize(buffer);
            if (errdecode.has_value())
            {
                std::cout << "Deserialize failed: " << errdecode.value().What() << std::endl;
                return;
            }
            if (err.has_value())
            {
                std::cout << "Error: " << err.value().What() << std::endl;
            }
            else
            {
                std::cout << "Received data: " << buffer.Size() << " bytes" << std::endl;
                std::cout << "Data: " << values[0].string << std::endl;
            }
        };

        client->RemoteCall("test_method", callback, INT32_MAX, INT64_MAX, 3, "helloworld");
    });

    io_thread->Start();
    io_thread->Join();
}