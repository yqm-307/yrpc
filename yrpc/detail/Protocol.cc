#include <yrpc/detail/Protocol.hpp>


namespace yrpc::detail
{

using namespace bbt::core::errcode;

ErrOpt Helper::ParseProtocolFromBuffer(bbt::core::Buffer& buffer, std::vector<bbt::core::Buffer>& protocols)
{
    // 协议解析
    while (true)
    {
        if (buffer.Size() < sizeof(ProtocolHead))
            break;

        ProtocolHead* head = (ProtocolHead*)buffer.Peek();

        if (head->protocol_length >= rpc_protocol_length_limit)
            return Errcode{"protocol length is invalid!", emErr::ERR_BAD_PROTOCOL_LENGTH_OVER_LIMIT};

        if (buffer.Size() < head->protocol_length)
            break;

        bbt::core::Buffer protocol{head->protocol_length};
        protocol.WriteNull(head->protocol_length);

        if (buffer.ReadString(protocol.Peek(), head->protocol_length))
            protocols.emplace_back(std::move(protocol));
        else
            return Errcode{"parse buffer failed!", emErr::ERR_BAD_PROTOCOL};
    }

    return std::nullopt;
}

} // namespace yrpc::detail