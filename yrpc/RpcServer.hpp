#pragma once
#include <yrpc/detail/Define.hpp>

namespace yrpc
{

class RpcServer
{
public:
    /**
     * @brief 构造一个监听ip和端口的TcpServer，并运行在io_thread上
     * 
     * @param io_thread 
     * @param ip 
     * @param port 
     */
    RpcServer(std::shared_ptr<bbt::network::EvThread> io_thread);
    ~RpcServer();

    bbt::core::errcode::ErrOpt Init(const char* ip, int port);

    /**
     * @brief rpc server开始运行，接收来自rpcclient的连接并处理remote call
     * 
     */
    void Run();

    /**
     * @brief 使Run返回
     * 
     */
    void Stop();

    bbt::core::errcode::ErrOpt RegisterMethod(const char* method_name, const RpcMethod& method);
    bbt::core::errcode::ErrOpt UnRegisterMethod(const char* method_name);
private:
    void _Accept(ConnId connid, short events);

    void OnRecv(ConnId connid, bbt::core::Buffer& buffer);
    void OnSend(ConnId connid, bbt::core::errcode::ErrOpt err, size_t send_len);
    void OnClose(ConnId connid);
    void OnTimeout(ConnId connid);
    void OnErr(const bbt::core::errcode::Errcode& err);
private:
    bbt::network::TcpServer* m_tcp_server{nullptr};

    std::mutex m_method_map_mutex;
    std::unordered_map<std::string, RpcMethod> m_method_map;
};


} // namespace yrpc