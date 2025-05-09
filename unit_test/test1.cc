#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MAIN
#include <boost/test/included/unit_test.hpp>

#include <bbt/core/thread/Lock.hpp>
#include <bbt/rpc/RpcServer.hpp>
#include <bbt/rpc/RpcClient.hpp>

using namespace bbt::rpc;
using namespace bbt::core;

BOOST_AUTO_TEST_SUITE()

BOOST_AUTO_TEST_CASE(t_rpc_remote_call)
{
    // 定义协议
    typedef bbt::rpc::RemoteCallTemplateRequest<int, int> AddReq;
    typedef bbt::rpc::RemoteCallTemplateReply<int> AddResp;


    auto io_thread = std::make_shared<bbt::network::EvThread>();
    auto server = std::make_shared<bbt::rpc::RpcServer>(io_thread);
    auto client = std::make_shared<bbt::rpc::RpcClient>(io_thread);

    // 服务端注册方法
    auto reg_err = server->RegisterMethod("add",
    [server](auto _, bbt::network::ConnId connid, bbt::rpc::RemoteCallSeq seq, const bbt::core::Buffer& data) {
        // std::string str;
        AddReq req;
        AddResp resp;
        Assert(codec::DeserializeWithTuple(data, req) == std::nullopt);

        int res = std::get<0>(req) + std::get<1>(req);
        resp = std::make_tuple(bbt::rpc::RPC_REPLY_TYPE_SUCCESS, res);

        BOOST_ASSERT(server->DoReply(connid, seq, resp) == std::nullopt);
        return std::nullopt;
    });

    BOOST_ASSERT(reg_err == std::nullopt);
    BOOST_ASSERT(server->Init("127.0.0.1", 8910, 10000) == std::nullopt);
    BOOST_ASSERT(client->Init("127.0.0.1", 8910, 1000, 10000) == std::nullopt);

    io_thread->Start();

    while (!client->IsConnected())
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    // 客户端发起调用
    bbt::core::thread::CountDownLatch l(1);
    AddReq req = std::make_tuple(100, 200);
    auto remote_call_err = client->RemoteCallWithTuple("add", 10000, req,
    [&](bbt::core::errcode::ErrOpt err, const bbt::core::Buffer& data)
    {
        BOOST_ASSERT(err == std::nullopt);

        AddResp resp;
        Assert(codec::DeserializeWithTuple(data, resp) == std::nullopt);
        BOOST_ASSERT(std::get<0>(resp) == bbt::rpc::RPC_REPLY_TYPE_SUCCESS);
        BOOST_ASSERT(std::get<1>(resp) == 300);
        l.Down();
    });

    BOOST_ASSERT(remote_call_err == std::nullopt);
    l.Wait();

    io_thread->Stop();
}

BOOST_AUTO_TEST_CASE(t_rpc_unregist_call)
{
    // 定义协议
    typedef bbt::rpc::RemoteCallTemplateRequest<int, int> AddReq;
    typedef bbt::rpc::RemoteCallTemplateReply<int> AddResp;

    auto io_thread = std::make_shared<bbt::network::EvThread>();
    auto server = std::make_shared<bbt::rpc::RpcServer>(io_thread);
    auto client = std::make_shared<bbt::rpc::RpcClient>(io_thread);

    // 服务端注册方法
    auto reg_err = server->RegisterMethod("add",
    [server](auto _, bbt::network::ConnId connid, bbt::rpc::RemoteCallSeq seq, const bbt::core::Buffer& data) {
        // std::string str;
        AddReq req;
        AddResp resp;
        Assert(codec::DeserializeWithTuple(data, req) == std::nullopt);

        int res = std::get<0>(req) + std::get<1>(req);
        resp = std::make_tuple(bbt::rpc::RPC_REPLY_TYPE_SUCCESS, res);

        BOOST_ASSERT(server->DoReply(connid, seq, resp) == std::nullopt);
        return std::nullopt;
    });

    BOOST_ASSERT(reg_err == std::nullopt);
    BOOST_ASSERT(server->Init("127.0.0.1", 8911, 1000000) == std::nullopt);
    BOOST_ASSERT(client->Init("127.0.0.1", 8911, 1000, 1000000) == std::nullopt);
    io_thread->Start();

    while (!client->IsConnected())
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }

    // 客户端发起调用
    bbt::core::thread::CountDownLatch l(1);
    AddReq req = std::make_tuple(100, 200);
    auto remote_call_err = client->RemoteCallWithTuple("add", 1000, req,
    [&](bbt::core::errcode::ErrOpt err, const bbt::core::Buffer& data)
    {
        BOOST_ASSERT(err == std::nullopt);

        AddResp resp;
        Assert(codec::DeserializeWithTuple(data, resp) == std::nullopt);
        BOOST_ASSERT(std::get<0>(resp) == bbt::rpc::RPC_REPLY_TYPE_SUCCESS);
        BOOST_ASSERT(std::get<1>(resp) == 300);
        l.Down();
    });

    BOOST_ASSERT(remote_call_err == std::nullopt);
    l.Wait();

    // 服务端注销协议
    l.Reset(1);
    BOOST_ASSERT(server->UnRegisterMethod("add") == std::nullopt);

    remote_call_err = client->RemoteCallWithTuple("add", 1000, req,
    [&](bbt::core::errcode::ErrOpt err, const bbt::core::Buffer& data)
    {
        BOOST_ASSERT(err.has_value());
        BOOST_TEST_MESSAGE("errinfo: " << err.value().What());
        l.Down();
    });
    
    l.Wait();

    io_thread->Stop();
}

BOOST_AUTO_TEST_CASE(t_rpc_call_timeout)
{
    // 定义协议
    typedef bbt::rpc::RemoteCallTemplateRequest<int, int> AddReq;
    typedef bbt::rpc::RemoteCallTemplateReply<int> AddResp;

    auto io_thread = std::make_shared<bbt::network::EvThread>();
    auto server = std::make_shared<bbt::rpc::RpcServer>(io_thread);
    auto client = std::make_shared<bbt::rpc::RpcClient>(io_thread);

    // 服务端注册方法
    server->RegisterMethod("add",
    [server](auto _, bbt::network::ConnId connid, bbt::rpc::RemoteCallSeq seq, const bbt::core::Buffer& data) {
        // std::string str;
        AddReq req;
        AddResp resp;
        Assert(codec::DeserializeWithTuple(data, req) == std::nullopt);

        int res = std::get<0>(req) + std::get<1>(req);
        resp = std::make_tuple(bbt::rpc::RPC_REPLY_TYPE_SUCCESS, res);

        // 不返回，让客户端超时
        // BOOST_ASSERT(server->DoReply(connid, seq, resp) == std::nullopt);
        return std::nullopt;
    });

    BOOST_ASSERT(server->Init("127.0.0.1", 8912, 1000000) == std::nullopt);
    BOOST_ASSERT(client->Init("127.0.0.1", 8912, 1000, 1000000) == std::nullopt);
    io_thread->Start();

    while (!client->IsConnected())
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }

    // 客户端发起调用
    bbt::core::thread::CountDownLatch l(1);
    AddReq req = std::make_tuple(100, 200);
    auto remote_call_err = client->RemoteCallWithTuple("add", 200, req,
    [&](bbt::core::errcode::ErrOpt err, const bbt::core::Buffer& data)
    {
        BOOST_ASSERT(err.has_value() && err->Type() == ERR_CLIENT_TIMEOUT);
        BOOST_TEST_MESSAGE("errinfo: " << err.value().What());
        l.Down();
    });

    BOOST_ASSERT(remote_call_err == std::nullopt);
    l.Wait();

    io_thread->Stop();
}

BOOST_AUTO_TEST_CASE(t_rpc_nocallback)
{
    // 定义协议
    typedef bbt::rpc::RemoteCallTemplateRequest<std::string> NotifyReq;
    bbt::core::thread::CountDownLatch l(1);

    auto io_thread = std::make_shared<bbt::network::EvThread>();
    auto server = std::make_shared<bbt::rpc::RpcServer>(io_thread);
    auto client = std::make_shared<bbt::rpc::RpcClient>(io_thread);

    // 服务端注册方法
    auto reg_err = server->RegisterMethod("notify",
    [&](auto _, bbt::network::ConnId connid, bbt::rpc::RemoteCallSeq seq, const bbt::core::Buffer& data) {
        NotifyReq req;
        Assert(codec::DeserializeWithTuple(data, req) == std::nullopt);
        auto str = std::get<0>(req);
        BOOST_CHECK_EQUAL("hello", str);
        l.Down();
        return std::nullopt;
    });

    BOOST_ASSERT(reg_err == std::nullopt);
    BOOST_ASSERT(server->Init("127.0.0.1", 8913, 1000000) == std::nullopt);
    BOOST_ASSERT(client->Init("127.0.0.1", 8913, 1000, 1000000) == std::nullopt);
    io_thread->Start();

    while (!client->IsConnected())
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }

    // 客户端发起调用
    NotifyReq req = std::make_tuple("hello");
    auto remote_call_err = client->RemoteCallWithTuple("notify", req);

    BOOST_ASSERT(remote_call_err == std::nullopt);
    l.Wait();

    io_thread->Stop();
}

BOOST_AUTO_TEST_SUITE_END()