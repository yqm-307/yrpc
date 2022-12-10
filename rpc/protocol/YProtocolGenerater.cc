#include "./YProtocolGenerater.h"

using namespace yrpc::detail::protocol;

YProtocolGenerater::YProtocolGenerater(const YProtocolGenerater &p)
    :m_protocol_head(p.m_protocol_head),
    m_message(p.m_message)
{
}


YProtocolGenerater::YProtocolGenerater(MessagePtr req,const ProtocolHead& head,YRPC_PROTOCOL prototype)
    :m_protocol_head(head),
    m_message(req)
{
    m_protocol_head.m_type = prototype;
}

// YProtocolGenerater::YProtocolGenerater()
//     : m_message(nullptr)
// {
// }

YProtocolGenerater::YProtocolGenerater(MessagePtr req,uint32_t serviceid, YRPC_PROTOCOL prototype)
    : m_message(req)
{
    m_protocol_head.m_serviceid = serviceid;
    m_protocol_head.m_type = prototype;
    assert(m_message);
}


YProtocolGenerater& YProtocolGenerater::operator=(const YProtocolGenerater &p)
{
    m_protocol_head = p.m_protocol_head;
    m_message = p.m_message;
    return *this;
}

bool YProtocolGenerater::ToByteArray(Buffer&bytearray) const
{
    char tmp[ProtocolHeadSize];
    memset(tmp,'0',ProtocolHeadSize);
    bytearray.InitAll();    // 情空
    bytearray.WriteString(tmp,ProtocolHeadSize);
    // bytearray.WriteString((const char*)head,ProtocolHeadSize);   // 先写入head

    assert(m_message!=nullptr);
    std::string tt;
    if (this->Encode(m_message,tt)) // protobuf 序列化
    {
        m_protocol_head.m_length = tt.size() + ProtocolHeadSize; // 协议长
        m_protocol_head.EnCode(bytearray.Peek());
        bytearray.WriteString(tt.c_str(),tt.size());
        return true;
    }
    return false;
}


