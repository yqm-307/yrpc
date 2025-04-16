#pragma once
#include <bbt/rpc/detail/Define.hpp>
#include <bbt/rpc/detail/RpcCodec.hpp>
#include <bbt/core/buffer/Buffer.hpp>

namespace bbt::rpc::detail
{

/**
 * @brief protocol 构成为
 * 
 * | ProtocolHead | byte[] |
 * 
 */

#pragma pack(push, 1)

struct ProtocolHead
{
    RpcMethodHash       method_hash;    // 调用的方法的hash
    RemoteCallSeq       call_seq;       // 此次调用的序号
    uint32_t            protocol_length;    // 协议长度
};

#pragma pack(pop)

typedef std::tuple<emRpcReplyType, std::string> RpcErrReply;

class Helper
{
public:
    /**
     * @brief 协议解析，从buffer中解析出完整的协议，并push到protocols中，解析成功的数据会从buffer中删除
     * 
     * @param buffer 待解析数据
     * @param protocols 解析后的协议
     * @return util::errcode::ErrOpt 
     */
    static bbt::core::errcode::ErrOpt ParseProtocolFromBuffer(bbt::core::Buffer& buffer, std::vector<bbt::core::Buffer>& protocols);

    template<typename ...Args>
    static void SerializeReq(bbt::core::Buffer& buffer, RpcMethodHash hash, RemoteCallSeq seq, Args&&... args)
    {
        static RpcCodec codec;
        ProtocolHead* head = nullptr;

        // protocol head
        buffer.WriteNull(sizeof(ProtocolHead));

        // protocol body
        codec.SerializeAppend(buffer, std::forward<Args>(args)...);

        head = (ProtocolHead*)buffer.Peek();
        head->method_hash = hash;
        head->call_seq = seq;
        head->protocol_length = buffer.Size();
    }

    template<typename Tuple>
    static void SerializeReqWithTuple(bbt::core::Buffer& buffer, RpcMethodHash hash, RemoteCallSeq seq, Tuple&& args)
    {
        static RpcCodec codec;
        ProtocolHead* head = nullptr;

        // protocol head
        buffer.WriteNull(sizeof(ProtocolHead));

        // protocol body
        // codec.SerializeAppend(buffer, RPC_REPLY_TYPE_SUCCESS);
        codec.SerializeAppendWithTuple(buffer, std::forward<Tuple>(args));

        head = (ProtocolHead*)buffer.Peek();
        head->method_hash = hash;
        head->call_seq = seq;
        head->protocol_length = buffer.Size();
    }

    /**
     * @brief 尝试从reply中获取一个Err
     * 
     * @param buffer 
     * @return ErrOpt 
     */
    static bbt::core::errcode::ErrOpt ReplyToErr(bbt::core::Buffer& buffer);
};

} // namespace bbt::rpc::detail