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
    m_acceptor->setOnAccept([this](const errorcode& err, Connection::SPtr conn){
        this->OnAccept(err, conn);
    });
}

void ChannelMgr::SetConnector(yrpc::detail::net::Connector::SPtr connector)
{
    m_connector = connector;
    m_connector->SetOnConnectCallback(
    [this](const yrpc::detail::shared::errorcode& err, Connection::SPtr new_conn, const yrpc::detail::net::YAddress& addr){
        OnConnect(err, new_conn, addr);
    });
}

void ChannelMgr::AsyncConnect(const yrpc::detail::net::YAddress& peer_addr)
{
    m_connector->AsyncConnect(peer_addr);
}

void ChannelMgr::InitAChannel(Channel::SPtr chan)
{
    chan->SetCloseCallback([this](const errorcode& err, Channel::SPtr chan){
        this->m_onclose(err, chan);
    });

    // chan->SetCloseCallback();
}

void ChannelMgr::OnConnect(const errorcode& err, Connection::SPtr conn, const yrpc::detail::net::YAddress& addr)
{
    Channel::SPtr chan_ptr = nullptr;
    if( err.err() == yrpc::detail::shared::ERR_NETWORK_CONN_OK )
    {
        chan_ptr = Channel::Create(conn);
        InitAChannel(chan_ptr);
        DEBUG("[YRPC][ChannelMgr::OnConnect][%d] on connect!", y_scheduler_id);
    }
    if( m_onconnect )
    {
        m_onconnect(err, chan_ptr, addr);
    }
    else
        DefaultOnConnect(err, conn);
}

void ChannelMgr::OnAccept(const errorcode& err, Connection::SPtr conn)
{
    Channel::SPtr chan_ptr = nullptr;
    if( err.err() == yrpc::detail::shared::ERR_NETWORK_ACCEPT_OK )
    {
        chan_ptr = Channel::Create(conn);
        InitAChannel(chan_ptr);
        DEBUG("[YRPC][ChannelMgr::OnAccept][%d] on accept!", y_scheduler_id);
    }
    if( m_onaccept )
    {
        m_onaccept(err, chan_ptr);
    }
    else
        DefaultOnAccept(err, conn);
}

void ChannelMgr::DefaultOnConnect(const errorcode& err, Connection::SPtr conn)
{
    if( err.err() != yrpc::detail::shared::ERR_NETWORK_CONN_OK )
    {
        ERROR("[YRPC][ChannelMgr::DefaultOnConnect][%d] on connect failed! msg: %s", y_scheduler_id, err.what().c_str());
    }
    else
    {
        INFO("[YRPC][ChannelMgr::DefaultOnConnect][%d] on connect. peer ip: {%s}", y_scheduler_id, conn->GetPeerAddress().GetIPPort());
        conn->Close();
    }
}

void ChannelMgr::DefaultOnAccept(const errorcode& err, Connection::SPtr conn)
{
    if( err.err() != yrpc::detail::shared::ERR_NETWORK_ACCEPT_OK )
    {
        ERROR("[YRPC][ChannelMgr::DefaultOnAccept][%d] on accept failed! msg: %s", y_scheduler_id, err.what().c_str());
    }
    else
    {
        INFO("[YRPC][ChannelMgr::DefaultOnAccept][%d] on accept. peer ip: {%s}", y_scheduler_id, conn->GetPeerAddress().GetIPPort());
        conn->Close();
    }
}

void ChannelMgr::DefaultOnClose(const errorcode& err, Connection::SPtr conn)
{
    INFO("[YRPC][ChannelMgr::DefaultOnClose][%d] on close! peer ip: {%s}", y_scheduler_id, conn->GetPeerAddress().GetIPPort());
}


void ChannelMgr::StartListen()
{
    assert(m_acceptor != nullptr);
    m_acceptor->StartListen();
}