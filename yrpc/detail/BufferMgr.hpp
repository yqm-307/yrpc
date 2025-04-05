#pragma once
#include <bbt/network/TcpServer.hpp>
#include <bbt/core/buffer/Buffer.hpp>
#include <bbt/core/errcode/Errcode.hpp>

namespace yrpc::detail
{

class BufferMgr
{
public:
    BufferMgr() = default;
    ~BufferMgr() = default;

    bbt::core::errcode::ErrOpt AddBuffer(bbt::network::ConnId connid, const bbt::core::Buffer& buffer);

    void RemoveBuffer(bbt::network::ConnId connid);

    std::optional<bbt::core::Buffer> GetBuffer(bbt::network::ConnId connid);

private:
    std::unordered_map<bbt::network::ConnId, bbt::core::Buffer> m_buffer_map;
};

} // namespace yrpc::detail