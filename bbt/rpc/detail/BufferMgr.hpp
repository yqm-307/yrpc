#pragma once
#include <bbt/network/TcpServer.hpp>
#include <bbt/core/buffer/Buffer.hpp>
#include <bbt/core/errcode/Errcode.hpp>

namespace bbt::rpc::detail
{

class BufferMgr
{
public:
    BufferMgr() = default;
    ~BufferMgr() = default;

    bbt::core::errcode::ErrOpt AddBuffer(bbt::network::ConnId connid, const bbt::core::Buffer& buffer);

    void RemoveBuffer(bbt::network::ConnId connid);

    bbt::core::Buffer* GetBuffer(bbt::network::ConnId connid);

    // 获取当前buffer的数量
    size_t Size() const;

    // 获取所有buffer的总字节数
    size_t GetTotalByte() const;

private:
    std::unordered_map<bbt::network::ConnId, bbt::core::Buffer> m_buffer_map;
};

} // namespace bbt::rpc::detail