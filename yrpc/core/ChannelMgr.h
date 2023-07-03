#include "Channel.h"

namespace yrpc::rpc::detail
{

class ChannelMgr
{
    typedef yrpc::detail::shared::errorcode         errorcode;
    typedef yrpc::detail::net::Connection           Connection;
    typedef std::function<void(const errorcode&, Channel::SPtr)> OnConnectCallback;
    typedef std::function<void(const errorcode&, Channel::SPtr)> OnAcceptCallback;
public:
    ChannelMgr();
    ~ChannelMgr();
    void SetAcceptor(yrpc::detail::net::Acceptor::SPtr acceptor);
    void SetConnector(yrpc::detail::net::Connector::SPtr connector);
    /* 注意初始化时机 */
    void SetOnConnect(const OnConnectCallback& cb)
    { m_onconnect = cb; }
    /* 注意初始化时机 */
    void SetOnAccept(const OnAcceptCallback& cb)
    { m_onaccept = cb; }
    void AsyncConnect(const yrpc::detail::net::YAddress& peer_addr);
    /* 需要先设置acceptor */
    void StartListen();
private:
    void OnConnect(const errorcode& err, Connection::SPtr conn);
    void OnAccept(const errorcode& err, Connection::SPtr conn);

    void DefaultOnConnect(const errorcode& err, Connection::SPtr conn);
    void DefaultOnAccept(const errorcode& err, Connection::SPtr conn);
private:
    yrpc::detail::net::Acceptor::SPtr     m_acceptor{nullptr};
    yrpc::detail::net::Connector::SPtr    m_connector{nullptr};
    OnConnectCallback m_onconnect;
    OnAcceptCallback    m_onaccept;
};



}