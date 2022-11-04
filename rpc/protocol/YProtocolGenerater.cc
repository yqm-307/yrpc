#include "./YProtocolGenerater.h"

using namespace yrpc::detail::protocol;

YProtocolGenerater::YProtocolGenerater(const YProtocolGenerater &p)
    : m_prototype(p.m_prototype),
      m_protocol_head(p.m_protocol_head),
      m_message(p.m_message)
{
}

// YProtocolGenerater::YProtocolGenerater()
//     : m_message(nullptr)
// {
// }

YProtocolGenerater::YProtocolGenerater(MessagePtr req, YRPC_PROTOCOL prototype)
    : m_prototype(prototype),
      m_message(req)
{
    assert(m_message);
}


YProtocolGenerater& YProtocolGenerater::operator=(const YProtocolGenerater &p)
{
    m_protocol_head = p.m_protocol_head;
    m_message = m_message;
    m_prototype = m_prototype;
}

bool YProtocolGenerater::ToByteArray(std::string &bytearray) const
{
    bytearray.resize(ProtocolHeadSize, '0'); // 预留 protocol head

    assert(m_message!=nullptr);
    if (this->Encode(m_message.get(), bytearray)) // 追加
    {
        m_protocol_head.m_length = bytearray.size(); // 协议长
        m_protocol_head.ToByteArray(bytearray.data());
        return true;
    }
    else
        return false;
}