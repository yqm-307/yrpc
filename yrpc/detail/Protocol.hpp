#pragma once
#include <yrpc/detail/Define.hpp>
#include <yrpc/detail/RpcCodec.hpp>
#include <bbt/core/buffer/Buffer.hpp>

namespace yrpc::detail
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
        RpcCodec codec;
        ProtocolHead* head = nullptr;

        buffer.WriteNull(sizeof(ProtocolHead));

        codec.SerializeAppend(buffer, std::forward<Args>(args)...);

        head = (ProtocolHead*)buffer.Peek();
        head->method_hash = hash;
        head->call_seq = seq;
        head->protocol_length = buffer.Size();
    }
};

} // namespace yrpc::detail