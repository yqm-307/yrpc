#include "RpcServer.h"
#include "yrpc/config/config.hpp"
using namespace yrpc::rpc;

RpcServer::RpcServer()
{
}

RpcServer::~RpcServer()
{
}


void RpcServer::SetAddress(Address&& addr)
{
    m_serv_addr = std::move(addr);
}   

void RpcServer::SetAddress(const Address& addr)
{
    m_serv_addr = addr;
}

void RpcServer::Start()
{
    yrpc::rpc::detail::__YRPC_SessionManager::GetInstance()->AsyncAccept(m_serv_addr);
}