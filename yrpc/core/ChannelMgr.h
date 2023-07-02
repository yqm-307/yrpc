#include "Channel.h"

namespace yrpc::rpc::detail
{

class ChannelMgr
{
    typedef yrpc::detail::shared::errorcode         errorcode;
    typedef yrpc::detail::net::Connection           Connection;
    typedef std::function<void(const errorcode&, Channel::SPtr)> OnConnectCallback;
public:
    ChannelMgr();
    ~ChannelMgr();
    void SetAcceptor(yrpc::detail::net::Acceptor::SPtr acceptor);
    void SetConnector(yrpc::detail::net::Connector::SPtr connector);
    void SetOnConnect(const OnConnectCallback& cb)
    { m_connected = cb; }
    void AsyncConnect(const yrpc::detail::net::YAddress& peer_addr);
private:
    void OnConnect(const errorcode& err, Connection::SPtr conn);
private:
    yrpc::detail::net::Acceptor::SPtr     m_acceptor;
    yrpc::detail::net::Connector::SPtr    m_connector;
    OnConnectCallback m_connected;
};



}