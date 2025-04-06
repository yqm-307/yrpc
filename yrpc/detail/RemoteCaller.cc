#include <yrpc/detail/Define.hpp>
#include <yrpc/detail/RemoteCaller.hpp>

using namespace bbt::core::errcode;
using namespace bbt::core::clock;
using namespace bbt::core;

namespace yrpc::detail
{

RemoteCaller::RemoteCaller(int timeout, RemoteCallSeq seq, const RpcReplyCallback& callback):
    m_timeout(nowAfter(ms(timeout > 0 ? timeout : INT32_MAX))),
    m_callback(callback),
    m_seq(seq)
{
}

RemoteCaller::~RemoteCaller()
{
}

bool RemoteCaller::operator<(const RemoteCaller& other) const
{
    return m_timeout < other.m_timeout;
}

Timestamp<> RemoteCaller::GetTimeout() const
{
    return m_timeout;
}

void RemoteCaller::TimeoutReply()
{
    bool expected = false;
    if (m_is_replyed.compare_exchange_strong(expected, true))
    {
        return;
    }

    if (m_callback)
    {
        m_callback(Errcode{"reply is timeout!", emErr::ERR_CLIENT_TIMEOUT}, Buffer{0});
    }
}

void RemoteCaller::SuccReply(const Buffer& buffer)
{
    bool expected = false;
    if (m_is_replyed.compare_exchange_strong(expected, true))
    {
        return;
    }

    if (m_callback)
    {
        m_callback(std::nullopt, buffer);
    }
}

void RemoteCaller::FailedReply(const Errcode& err)
{
    bool expected = false;
    if (m_is_replyed.compare_exchange_strong(expected, true))
    {
        return;
    }

    if (m_callback)
    {
        m_callback(err, Buffer{0});
    }
}

RemoteCallSeq RemoteCaller::GetSeq() const
{
    return m_seq;
}

bool RemoteCaller::IsReplyed() const
{
    return m_is_replyed.load();
}

} // namespace yrpc::detail