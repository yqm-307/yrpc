#include <bbt/rpc/detail/Protocol.hpp>
#include <bbt/rpc/detail/RpcCodec.hpp>


namespace bbt::rpc::detail
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

bbt::core::errcode::ErrOpt Helper::ReplyToErr(bbt::core::Buffer& buffer)
{
    static RpcCodec codec;
    RpcErrReply err_tuple;
    std::tuple<emRpcReplyType> err_type;

    // 服务器的reply返回必须带有类型
    if (auto err = codec.DeserializeWithTuple(buffer, err_type); err.has_value())
        return Errcode{"[bbt::rpc] service reply-data has no RpcReplyType!", emErr::ERR_BAD_PROTOCOL};

    if (std::get<0>(err_type) != RPC_REPLY_TYPE_FAILED) {
        return std::nullopt;
    }

    // 服务器的err reply格式
    if (auto err = codec.DeserializeWithTuple(buffer, err_tuple); err.has_value())
        return Errcode{"[bbt::rpc] service reply-data err type is bad!", emErr::ERR_BAD_PROTOCOL};

    // 解析正确的错误类型
    std::string err_msg = std::get<1>(err_tuple);

    buffer.ReadNull(sizeof(FieldHeader) * 2 + sizeof(emRpcReplyType) + err_msg.size());
    return Errcode{std::get<1>(err_tuple), emErr::ERR_CLIENT_FAILED};
}

} // namespace bbt::rpc::detail