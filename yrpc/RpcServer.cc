#include <yrpc/RpcServer.hpp>
#include <tuple>

using namespace bbt::core::errcode;
using namespace bbt::network;

namespace yrpc
{

RpcServer::RpcServer(std::shared_ptr<bbt::network::EvThread> io_thread)
{
    m_tcp_server = new bbt::network::TcpServer(io_thread);
    m_tcp_server->Init();
    m_tcp_server->AsyncListen(bbt::core::net::IPAddress(ip, port), std::bind(&RpcServer::_Accept, this, std::placeholders::_1, std::placeholders::_2));
}

RpcServer::~RpcServer()
{
    Stop();
    delete m_tcp_server;

    m_tcp_server = nullptr;
}

ErrOpt RpcServer::Init(const char* ip, int port)
{
    if (m_tcp_server == nullptr)
    {
        return Errcode{"tcp server is null!", ERR_COMM};
    }

    m_tcp_server->AsyncListen(bbt::core::net::IPAddress(ip, port), [this](ConnId connid){

    });
    return std::nullopt;
}


void RpcServer::Run()
{
}

void RpcServer::Stop()
{
}

ErrOpt RpcServer::RegisterMethod(const char* method_name, const RpcMethod& method)
{
    std::lock_guard<std::mutex> lock(m_method_map_mutex);
    auto it = m_method_map.find(method_name);
    if (it != m_method_map.end())
    {
        return Errcode{"repeat regist method!", ERR_METHOD_ALREADY_REGISTERED};
    }

    m_method_map[method_name] = method;
    return std::nullopt;
}

ErrOpt RpcServer::UnRegisterMethod(const char* method_name)
{
    std::lock_guard<std::mutex> lock(m_method_map_mutex);
    auto it = m_method_map.find(method_name);
    if (it == m_method_map.end())
    {
        return Errcode{"repeat regist method!", ERR_COMM};
    }

    m_method_map.erase(it);
    return std::nullopt;
}

}// namespace yrpc