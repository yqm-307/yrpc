#pragma once
#include <boost/operators.hpp>

#include <bbt/core/clock/Clock.hpp>

#include <yrpc/detail/Define.hpp>

namespace yrpc::detail
{

class RemoteCaller:
    public boost::less_than_comparable1<RemoteCaller>
{
public:
    RemoteCaller(int timeout, RemoteCallSeq seq, const RpcReplyCallback& callback);
    ~RemoteCaller();

    bbt::core::clock::Timestamp<> GetTimeout() const;

    bool operator<(const RemoteCaller& other) const;

    void TimeoutReply();
    void SuccReply(const bbt::core::Buffer& buffer);
    void FailedReply(const bbt::core::errcode::Errcode& err);
    RemoteCallSeq GetSeq() const;
private:
    bbt::core::clock::Timestamp<>   m_timeout;
    RpcReplyCallback                m_callback{nullptr};
    std::atomic_bool                m_is_replyed{false};
    const RemoteCallSeq             m_seq{0};
};

struct RemoteCallerComp
{
    bool operator()(const std::shared_ptr<RemoteCaller>& lhs, const std::shared_ptr<RemoteCaller>& rhs) const
    {
        return *lhs < *rhs;
    }
};

} // namespace yrpc::detail