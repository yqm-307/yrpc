#include <bbt/rpc/detail/Define.hpp>
#include <bbt/rpc/detail/BufferMgr.hpp>

using namespace bbt::core::errcode;

namespace bbt::rpc::detail
{

ErrOpt BufferMgr::AddBuffer(bbt::network::ConnId connid, const bbt::core::Buffer& buffer)
{
    if (m_buffer_map.find(connid) != m_buffer_map.end())
        return Errcode("Buffer already exists", emErr::ERR_COMM);

    m_buffer_map[connid] = buffer;
    return std::nullopt;
}

void BufferMgr::RemoveBuffer(bbt::network::ConnId connid)
{
    m_buffer_map.erase(connid);
}

bbt::core::Buffer* BufferMgr::GetBuffer(bbt::network::ConnId connid)
{
    auto it = m_buffer_map.find(connid);
    if (it == m_buffer_map.end())
        return nullptr;

    return &it->second;
}

size_t BufferMgr::Size() const
{
    return m_buffer_map.size();
}

size_t BufferMgr::GetTotalByte() const
{
    size_t total = 0;
    for (const auto& pair : m_buffer_map)
    {
        total += pair.second.Size();
    }
    return total;
}



} // namespace bbt::rpc::detail