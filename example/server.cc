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
        if (values.size() != 4)
        {
            std::cout << "Invalid number of arguments" << std::endl;
            return;
        }
        for (auto& value : values)
        {
            if (value.header.field_type == bbt::rpc::detail::FieldType::INT32)
                std::cout << "Field Type: " << (int)value.header.field_type << ", Length: " << value.header.field_len << ", Value:" << value.value.int32_value << std::endl;
            else if (value.header.field_type == bbt::rpc::detail::FieldType::INT64)
                std::cout << "Field Type: " << (int)value.header.field_type << ", Length: " << value.header.field_len << ", Value:" << value.value.int64_value << std::endl;
            else if (value.header.field_type == bbt::rpc::detail::FieldType::UINT32)
                std::cout << "Field Type: " << (int)value.header.field_type << ", Length: " << value.header.field_len << ", Value:" << value.value.uint32_value << std::endl;
            else if (value.header.field_type == bbt::rpc::detail::FieldType::UINT64)
                std::cout << "Field Type: " << (int)value.header.field_type << ", Length: " << value.header.field_len << ", Value:" << value.value.uint64_value << std::endl;
            else if (value.header.field_type == bbt::rpc::detail::FieldType::STRING)
                std::cout << "Field Type: " << (int)value.header.field_type << ", Length: " << value.header.field_len << ", Value:" << value.string << std::endl;
            
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