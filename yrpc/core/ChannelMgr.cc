#include "ChannelMgr.h"

using namespace yrpc::rpc::detail;

ChannelMgr::ChannelMgr(Epoller* main_loop)
    :m_main_loop(main_loop)
{
    // yrpc::detail::net::Connector::Create(main_loop);
}

ChannelMgr::~ChannelMgr()
{
}

void ChannelMgr::AsyncConnect(const yrpc::detail::net::YAddress& peer_addr)
{
    if( m_connector == nullptr )
    {
        InitConnector();
    }
    m_connector->AsyncConnect(peer_addr);
}

void ChannelMgr::InitConnector()
{
    assert(m_loadblancer != nullptr);
    m_connector = yrpc::detail::net::Connector::Create(m_main_loop);
    m_connector->SetLoadBalancer(m_loadblancer);
    m_connector->SetOnConnectCallback(
    [this](const yrpc::detail::shared::errorcode& err, Connection::UQPtr new_conn, const yrpc::detail::net::YAddress& addr){
        OnConnect(err, std::move(new_conn), addr);
    });
}

void ChannelMgr::InitAcceptor(const Address& listen_addr)
{
    assert(m_loadblancer != nullptr);
    m_acceptor = yrpc::detail::net::Acceptor::Create(m_main_loop, listen_addr.GetPort(), 3000, 3000);
    m_acceptor->SetLoadBalancer(m_loadblancer);
    m_acceptor->SetOnAccept([this](const errorcode& err, Connection::UQPtr conn){
        this->OnAccept(err, std::move(conn));
    });
}

void ChannelMgr::InitAChannel(Channel::SPtr chan)
{
    chan->SetCloseCallback([this](const errorcode& err, Channel::SPtr chan){
        this->m_onclose(err, chan);
    });
}

void ChannelMgr::OnConnect(const errorcode& err, Connection::UQPtr conn, const yrpc::detail::net::YAddress& addr)
{
    Channel::SPtr chan_ptr = Channel::Create(std::move(conn));
    if( err.err() == yrpc::detail::shared::ERR_NETWORK_CONN_OK )
    {
        DEBUG("[YRPC][ChannelMgr::OnConnect][%d] on connect!", y_scheduler_id);
    }
    if( m_onconnect )
    {
        m_onconnect(err, chan_ptr, addr);
    }
    else
        DefaultOnConnect(err, chan_ptr);
}

void ChannelMgr::OnAccept(const errorcode& err, Connection::UQPtr conn)
{
    Channel::SPtr chan_ptr = nullptr;
    if( err.err() == yrpc::detail::shared::ERR_NETWORK_ACCEPT_OK )
    {
        chan_ptr = Channel::Create(std::move(conn));
        InitAChannel(chan_ptr);
        DEBUG("[YRPC][ChannelMgr::OnAccept][%d] on accept! peer:{%s}", y_scheduler_id, conn->GetPeerAddress().GetIPPort().c_str());
    }
    if( m_onaccept )
    {
        m_onaccept(err, chan_ptr);
    }
    else
        DefaultOnAccept(err, std::move(conn));
}

void ChannelMgr::DefaultOnConnect(const errorcode& err, Channel::SPtr chan)
{
    if( err.err() != yrpc::detail::shared::ERR_NETWORK_CONN_OK )
    {
        ERROR("[YRPC][ChannelMgr::DefaultOnConnect][%d] on connect failed! msg: %s", y_scheduler_id, err.what().c_str());
    }
    else
    {
        INFO("[YRPC][ChannelMgr::DefaultOnConnect][%d] on connect. peer ip: {%s}", y_scheduler_id, chan->GetPeerAddress().GetIPPort());
        chan->Close();
    }
}

void ChannelMgr::DefaultOnAccept(const errorcode& err, Connection::UQPtr conn)
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

ChannelMgr::SPtr ChannelMgr::Create(Epoller* main_loop)
{
    return std::make_shared<ChannelMgr>(main_loop);
}

void ChannelMgr::StartListen(const Address& listen_addr)
{
    if(m_acceptor == nullptr)
    {
        InitAcceptor(listen_addr);
    }
    assert(m_loadblancer != nullptr);
    m_acceptor->SetLoadBalancer(m_loadblancer);
    m_acceptor->StartListen();
}