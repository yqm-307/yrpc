#include "./YProtocolResolver.h"

using namespace yrpc::detail::protocol;

YProtocolResolver::YProtocolResolver(const YProtocolResolver &p)
    : m_prototype(p.m_prototype),
      m_protocol_head(p.m_protocol_head)
{
}



YProtocolResolver::~YProtocolResolver() 
{
}



bool YProtocolResolver::ToProtoMsg(MessagePtr message) const
{
    m_protocol_head.SetByteArray(m_bytes.data());
    std::string_view proto_bytes(m_bytes.data(), m_bytes.size() - sizeof(uint16_t) * 3);
    return yrpc::detail::Codec::ParseToMessage(message, proto_bytes);
}