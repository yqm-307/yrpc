#include "ChannelMgr.h"

using namespace yrpc::rpc::detail;

ChannelMgr::ChannelMgr()
{
}

ChannelMgr::~ChannelMgr()
{
}

void ChannelMgr::SetAcceptor(yrpc::detail::net::Acceptor::SPtr acceptor)
{
    m_acceptor = acceptor;
}

void ChannelMgr::SetConnector(yrpc::detail::net::Connector::SPtr connector)
{
    m_connector = connector;
    m_connector->SetOnConnectCallback(functor([this](const yrpc::detail::shared::errorcode& err, Connection::SPtr new_conn){
        OnConnect(err, new_conn);
    }));
}

void ChannelMgr::OnConnect(const errorcode& err, Connection::SPtr conn)
{
    auto channel_ptr = Channel::Create(conn);
    m_connected(err, channel_ptr);
}

void ChannelMgr::AsyncConnect(const yrpc::detail::net::YAddress& peer_addr)
{
    m_connector->AsyncConnect(peer_addr);
}