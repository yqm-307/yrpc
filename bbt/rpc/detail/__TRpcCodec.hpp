#pragma once
#include <bbt/rpc/detail/RpcCodec.hpp>

namespace bbt::rpc::codec
{

template<typename T>
inline FieldType ToFieldType()
{
    if constexpr (std::is_same_v<T, int32_t>)
        return INT32;
    else if constexpr (std::is_same_v<T, uint32_t>)
        return UINT32;
    else if constexpr (std::is_same_v<T, int64_t>)
        return INT64;
    else if constexpr (std::is_same_v<T, uint64_t>)
        return UINT64;
    else if constexpr (std::is_same_v<T, int16_t>)
        return INT16;
    else if constexpr (std::is_same_v<T, uint16_t>)
        return UINT16;
    else if constexpr (std::is_same_v<T, int8_t>)
        return INT8;
    else if constexpr (std::is_same_v<T, uint8_t>)
        return UINT8;
    else if constexpr (std::is_same_v<T, std::string>)
        return STRING;
    else if constexpr (std::is_pod_v<T>)
        return POD;
    else
        return BYTEOBJ;
}

template<typename ...Args>
inline bbt::core::Buffer Serialize(Args... args)
{
    bbt::core::Buffer buffer;
    SerializeArgs(buffer, args...);
    return buffer;
}

template<typename Tuple>
inline bbt::core::Buffer SerializeWithTuple(Tuple&& tuple)
{
    bbt::core::Buffer buffer;

    std::apply([&](auto&&... args) {
        SerializeArgs(buffer, args...);
    }, std::forward<Tuple>(tuple));

    return buffer;
}

template<typename ...Args>
inline void SerializeAppend(bbt::core::Buffer& buffer, Args... args)
{
    SerializeArgs(buffer, args...);
}

template<typename Tuple>
inline void SerializeAppendWithTuple(bbt::core::Buffer& buffer, Tuple&& args)
{
    std::apply([&](auto&&... args) {
        SerializeArgs(buffer, args...);
    }, std::forward<Tuple>(args));
}

template<typename... Args>
inline bbt::core::errcode::ErrOpt DeserializeWithTuple(const bbt::core::Buffer& buffer, std::tuple<Args...>& args)
{
    size_t offset = 0;
    return DeserializeArgsRecursive(buffer, offset, args, std::index_sequence_for<Args...>{});
}


template<typename Tuple, size_t... Index>
inline bbt::core::errcode::ErrOpt DeserializeArgsRecursive(const bbt::core::Buffer& buffer, size_t& offset, Tuple& args, std::index_sequence<Index...>)
{
    bbt::core::errcode::ErrOpt err;
    // 递归展开每个参数
    ((err = DeserializeOneArg(buffer, offset, std::get<Index>(args))) || ...);
    return err;
}

template<typename T>
inline bbt::core::errcode::ErrOpt DeserializeOneArg(const bbt::core::Buffer& buffer, size_t& offset, T& arg)
{
    FieldHeader header;

    // 检查类型是否匹配
    static_assert(IsSupportType<T>::value, "Unsupported type for deserialization!");
    if (!buffer.ToString(offset, (char*)&header, sizeof(header)))
        return bbt::core::errcode::Errcode(BBT_RPC_ERR_PREFIX"deserialize failed, buffer too short!", emErr::ERR_COMM);
    offset += sizeof(header);

    if (header.field_type != ToFieldType<T>())
        return bbt::core::errcode::Errcode(BBT_RPC_ERR_PREFIX"deserialize failed, field type mismatch! expected type=" + std::to_string(ToFieldType<T>()) + " , but it is actually=" + std::to_string(header.field_type), emErr::ERR_COMM);

    if constexpr (std::is_same_v<T, std::string>)
    {
        arg.resize(header.field_len);
        if (!buffer.ToString(offset, arg.data(), header.field_len))
            return bbt::core::errcode::Errcode(BBT_RPC_ERR_PREFIX"deserialize failed, buffer too short!", emErr::ERR_COMM);
    }
    else {
        if (header.field_len != sizeof(arg))
            return bbt::core::errcode::Errcode(BBT_RPC_ERR_PREFIX"deserialize failed, field length mismatch!", emErr::ERR_COMM);

        if (!buffer.ToString(offset, (char*)&arg, sizeof(arg)))
            return bbt::core::errcode::Errcode(BBT_RPC_ERR_PREFIX"deserialize failed, buffer too short!", emErr::ERR_COMM);
    }
    offset += header.field_len;

    return std::nullopt;
}

template<typename T>
inline void SerializeArg(bbt::core::Buffer& bytes, T arg)
{
    FieldHeader header;
    header.field_type = ToFieldType<T>();
    header.field_len = sizeof(arg);
    bytes.WriteString((char*)&header, sizeof(header));
    bytes.Write(arg);
}

inline void SerializeArg(bbt::core::Buffer& buffer, const std::string& arg)
{
    FieldHeader header;
    header.field_type = STRING;
    header.field_len = arg.size();
    buffer.WriteString((char*)&header, sizeof(header));
    buffer.WriteString(arg.c_str(), arg.size());
}

inline void SerializeArg(bbt::core::Buffer& buffer, const char* arg)
{
    FieldHeader header;
    header.field_type = STRING;
    header.field_len = strlen(arg);
    buffer.WriteString((char*)&header, sizeof(header));
    buffer.WriteString(arg, strlen(arg));
}

inline void SerializeArgs(bbt::core::Buffer& buffer) {}

template<typename T, typename... Args>
inline void SerializeArgs(bbt::core::Buffer& buffer, T arg, Args... args)
{
    SerializeArg(buffer, arg);
    SerializeArgs(buffer, args...);
}



} // namespace bbt::rpc::detail