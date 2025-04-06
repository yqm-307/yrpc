#include <yrpc/RpcClient.hpp>
#include <bbt/pollevent/Event.hpp>

void Monitor(std::shared_ptr<yrpc::RpcClient> client)
{
    std::cout << "Monitor" << std::endl;
    std::cout << client->DebugInfo() << std::endl;
}

int main()
{
    std::atomic<int> count{0};

    auto evloop = std::make_shared<bbt::pollevent::EventLoop>();
    auto io_thread = std::make_shared<bbt::network::EvThread>(evloop);

    auto client = std::make_shared<yrpc::RpcClient>(io_thread);
    if (auto err = client->Init("", 10031, 3000); err.has_value())
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
            auto err = client->RemoteCall("echo", 1000, [&count](bbt::core::errcode::ErrOpt err, const bbt::core::Buffer& buf){ count += buf.Size(); }, "hello world");
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