#include <bbt/rpc/RpcClient.hpp>
#include <bbt/pollevent/Event.hpp>

void Monitor(std::shared_ptr<bbt::rpc::RpcClient> client)
{
    std::cout << "Monitor" << std::endl;
    std::cout << client->DebugInfo() << std::endl;
}

int main(int argc, char* argv[])
{
    if (argc < 3)
    {
        std::cout << "Usage: " << argv[0] << " <ip> <port>" << std::endl;
        return -1;
    }

    std::string ip = argv[1];
    int port = std::stoi(argv[2]);

    std::atomic<int> count{0};

    auto evloop = std::make_shared<bbt::pollevent::EventLoop>();
    auto io_thread = std::make_shared<bbt::network::EvThread>(evloop);

    auto client = std::make_shared<bbt::rpc::RpcClient>(io_thread);
    if (auto err = client->Init(ip.c_str(), port, 3000, 4000); err.has_value())
    {
        std::cout << "Init failed: " << err.value().What() << std::endl;
        return -1;
    }

    auto monitor_event = io_thread->RegisterEvent(-1, bbt::pollevent::EventOpt::PERSIST, [client](int fd, short events, bbt::pollevent::EventId id) {
        Monitor(client);
    });
    monitor_event->StartListen(1000);

    io_thread->Start();
    // 等连接建立
    sleep(1);

    while (true)
    {
        for (int i = 0; i < 10000; ++i)
        {
            auto err = client->RemoteCallWithTuple("echo", 1000, std::make_tuple("hello world"), [&count](bbt::core::errcode::ErrOpt err, const bbt::core::Buffer& buf){ count += buf.Size(); });
            if (err.has_value())
            {
                std::cout << "RemoteCall failed: " << err.value().What() << std::endl;
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }




    sleep(1000);
    io_thread->Stop();
    
}