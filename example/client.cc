#include <bbt/rpc/RpcClient.hpp>
#include "proto.hpp"

using namespace bbt::rpc;

class MyClient : public bbt::rpc::RpcClient
{
public:
    MyClient(std::shared_ptr<bbt::network::EvThread> io_thread):
        bbt::rpc::RpcClient(io_thread)
    {}

    void OnError(const bbt::core::errcode::Errcode& err)
    {
        std::cout << "OnError: " << err.What() << std::endl;
    }

    void OnTimeout(bbt::network::ConnId) override
    {
        std::cout << "OnTimeout: " << std::endl;
    }
private:
};

int main()
{
    auto evloop = std::make_shared<bbt::pollevent::EventLoop>();
    auto io_thread = std::make_shared<bbt::network::EvThread>(evloop);

    auto client = std::make_shared<MyClient>(io_thread);

    client->Init("127.0.0.1", 10031, 1000, 10000, [](std::shared_ptr<bbt::rpc::RpcClient> client) {
        std::cout << "Connected to server!" << std::endl;

        auto callback = [](bbt::core::errcode::ErrOpt err, const bbt::core::Buffer& buffer) {
            if (err.has_value())
            {
                std::cout << "RemoteCall failed: " << err.value().What() << std::endl;
                return;
            }

            auto tuple = std::make_tuple(std::string{buffer.Peek(), buffer.Size()});
            auto errdecode = codec::DeserializeWithTuple(buffer, tuple);
            if (errdecode.has_value())
            {
                std::cout << "Deserialize failed: " << errdecode.value().What() << std::endl;
                return;
            }
            std::cout << "Received: " << std::get<0>(tuple) << std::endl;
        };

        // 正确参数调用
        if (auto err = client->RemoteCallWithTuple("test_method", 1000, std::make_tuple(INT32_MAX, INT64_MAX, 3, "helloworld"), callback); err.has_value())
            std::cout << "RemoteCall failed 1: " << err.value().What() << std::endl;

        // 错误参数调用
        if (auto err = client->RemoteCallWithTuple("test_method", 1000, std::make_tuple(INT32_MAX, INT64_MAX, 3, 112, "helloworld"), callback); err.has_value())
            std::cout << "RemoteCall failed 2: " << err.value().What() << std::endl;

        std::cout << "-- do call with tuple --" << std::endl;
        auto tuple = std::make_tuple(100, (int64_t)10, 3, "helloworld");
        if (auto err = client->RemoteCallWithTuple("test_method", 1000, tuple, callback); err.has_value())
            std::cout << "RemoteCall failed 4: " << err.value().What() << std::endl;

        if (auto err = client->RemoteCallWithTuple("bad call", 1000, std::make_tuple(""), nullptr); err.has_value())
        {
            std::cout << "RemoteCall failed 3: " << err.value().What() << std::endl;
        }

    });

    io_thread->Start();
    io_thread->Join();
}