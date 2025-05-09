#include <bbt/rpc/RpcServer.hpp>
#include <bbt/pollevent/Event.hpp>

using namespace bbt::rpc;
using namespace bbt::core;

void Echo(std::shared_ptr<bbt::rpc::RpcServer> server, bbt::network::ConnId connid, bbt::rpc::RemoteCallSeq seq, const bbt::core::Buffer& data)
{
    std::tuple<std::string> values;
    auto err = codec::DeserializeWithTuple(data, values);
    if (err.has_value())
    {
        std::cout << "Deserialize failed: " << err.value().What() << std::endl;
        return;
    }


    server->DoReply(connid, seq, std::make_tuple(std::get<0>(values)));
}

void Monitor(std::shared_ptr<bbt::rpc::RpcServer> server)
{
    // std::cout << "Monitor: " << data.ToString() << std::endl;
    std::cout << "Monitor" << std::endl;
    std::cout << server->DebugInfo() << std::endl;
}

int main(int argc, char* argv[])
{
    if (argc < 2)
    {
        std::cout << "Usage: " << argv[0] << " <ip> <port>" << std::endl;
        return -1;
    }

    std::string ip = argv[1];
    int port = std::stoi(argv[2]);

    auto evloop = std::make_shared<bbt::pollevent::EventLoop>();
    auto io_thread = std::make_shared<bbt::network::EvThread>(evloop);

    auto server = std::make_shared<bbt::rpc::RpcServer>(io_thread);

    if (auto err = server->Init(ip.c_str(), port, 3000); err.has_value())
    {
        std::cout << "Init failed: " << err.value().What() << std::endl;
        return -1;
    }

    if (auto err = server->RegisterMethod("echo", [](std::shared_ptr<bbt::rpc::RpcServer> server, bbt::network::ConnId connid, bbt::rpc::RemoteCallSeq seq, const bbt::core::Buffer& data) {
        Echo(server, connid, seq, data);
        return std::nullopt;
    }); err.has_value())
    {
        std::cout << "RegisterMethod failed: " << err.value().What() << std::endl;
        return -1;
    }

    auto monitor_event = io_thread->RegisterEvent(-1, bbt::pollevent::EventOpt::PERSIST, [server](int fd, short events, bbt::pollevent::EventId id) {
        Monitor(server);
    });
    monitor_event->StartListen(1000);

    io_thread->Start();
    sleep(1200);
    io_thread->Stop();
}