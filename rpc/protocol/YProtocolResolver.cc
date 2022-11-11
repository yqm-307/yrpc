#include "./YProtocolResolver.h"

using namespace yrpc::detail::protocol;






YProtocolResolver::YProtocolResolver(std::string_view bytes):m_bytes(bytes)
{
    ParseHead();
}

YProtocolResolver::YProtocolResolver(const YProtocolResolver &p)
    :m_protocol_head(p.m_protocol_head),
    m_prototype(p.m_prototype)
{
}



YProtocolResolver::~YProtocolResolver() 
{
}



bool YProtocolResolver::ToProtoMsg(MessagePtr message) const
{
    // m_protocol_head.DeCode(m_bytes.data());
    std::string_view proto_bytes(m_bytes.data()+ProtocolHeadSize, m_bytes.size()-(size_t)(ProtocolHeadSize));
    return yrpc::detail::Codec::ParseToMessage(message, proto_bytes);
}

void YProtocolResolver::ParseHead()
{
    m_protocol_head.DeCode(m_bytes.c_str());
}