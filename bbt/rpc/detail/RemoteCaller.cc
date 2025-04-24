#include <bbt/rpc/detail/Define.hpp>
#include <bbt/rpc/detail/RemoteCaller.hpp>
#include <bbt/rpc/detail/Protocol.hpp>

using namespace bbt::core::errcode;
using namespace bbt::core::clock;
using namespace bbt::core;

namespace bbt::rpc::detail
{

RemoteCaller::RemoteCaller(int timeout, RemoteCallSeq seq, const RpcReplyCallback& callback):
    m_timeout(nowAfter(ms(timeout > 0 ? timeout : INT32_MAX))),
    m_callback(callback),
    m_seq(seq),
    m_type(callback ? emRemoteCallType::TIMEOUT_REPLY : emRemoteCallType::ONLY_REQ)
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

void RemoteCaller::Reply(bbt::core::Buffer& buffer, bbt::core::errcode::ErrOpt err)
{
    bool expected = false;
    if (!m_is_replyed.compare_exchange_strong(expected, true))
    {
        return;
    }

    if (!m_callback)
        return;

    if (err.has_value())
    {
        m_callback(err, Buffer{0});
        return;
    }

    err = Helper::ReplyToErr(buffer);
    if (err.has_value())
        m_callback(err, Buffer{0});
    else
        m_callback(std::nullopt, buffer);
}

RemoteCallSeq RemoteCaller::GetSeq() const
{
    return m_seq;
}

bool RemoteCaller::IsReplyed() const
{
    return m_is_replyed.load();
}

emRemoteCallType RemoteCaller::GetType() const
{
    return m_type;
}

} // namespace bbt::rpc::detail