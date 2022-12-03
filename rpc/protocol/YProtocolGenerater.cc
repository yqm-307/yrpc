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
    std::string tmp{""};
    tmp.resize(ProtocolHeadSize,'0');
    // bytearray.WriteString((const char*)head,ProtocolHeadSize);   // 先写入head

    assert(m_message!=nullptr);
    if (this->Encode(m_message.get(), tmp)) // 追加
    {
        m_protocol_head.m_length = tmp.size(); // 协议长
        m_protocol_head.EnCode(tmp.data());
        bytearray = Buffer(tmp);
        return true;
    }
    else
        return false;
}

