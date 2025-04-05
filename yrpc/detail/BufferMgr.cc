#include <yrpc/detail/Define.hpp>
#include <yrpc/detail/BufferMgr.hpp>

using namespace bbt::core::errcode;

namespace yrpc::detail
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

std::optional<bbt::core::Buffer> BufferMgr::GetBuffer(bbt::network::ConnId connid)
{
    auto it = m_buffer_map.find(connid);
    if (it == m_buffer_map.end())
        return std::nullopt;

    return it->second;
}



} // namespace yrpc::detail