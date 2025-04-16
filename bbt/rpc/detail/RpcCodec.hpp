#pragma once
#include <bbt/rpc/detail/Define.hpp>


namespace bbt::rpc::detail
{

/**
 * @brief Codec 负责二进制序列化和反序列化
 * 
 * 二进制格式为下面格式的数组
 * 
 * | field_type | field_len | byte[] |
 * 
 */

enum FieldType : int8_t
{
    INT64 = 1,
    UINT64,
    INT32,
    UINT32,
    STRING,
    BYTEOBJ, // 字节对象
};

#pragma pack(push, 1)

struct FieldHeader
{
    FieldType field_type;
    int16_t field_len;
};

#pragma pack(pop)




template<typename T> struct IsSupportType { static constexpr bool value = true; };

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
    else if constexpr (std::is_same_v<T, std::string>)
        return STRING;
    else
        return BYTEOBJ;
}

class RpcCodec
{
public:
    /**
     * @brief 将args打包为字节流并返回
     * 
     * @tparam Args 
     * @param args 
     * @return bbt::core::Buffer 
     */
    template<typename ...Args>
    bbt::core::Buffer Serialize(Args... args)
    {
        bbt::core::Buffer buffer;
        SerializeArgs(buffer, args...);
        return buffer;
    }

    template<typename Tuple>
    bbt::core::Buffer SerializeWithTuple(Tuple&& tuple)
    {
        bbt::core::Buffer buffer;

        std::apply([&](auto&&... args) {
            SerializeArgs(buffer, args...);
        }, std::forward<Tuple>(tuple));

        return buffer;
    }

    template<typename ...Args>
    void SerializeAppend(bbt::core::Buffer& buffer, Args... args)
    {
        SerializeArgs(buffer, args...);
    }

    template<typename Tuple>
    void SerializeAppendWithTuple(bbt::core::Buffer& buffer, Tuple&& args)
    {
        std::apply([&](auto&&... args) {
            SerializeArgs(buffer, args...);
        }, std::forward<Tuple>(args));
    }

    template<typename... Args>
    bbt::core::errcode::ErrOpt DeserializeWithTuple(const bbt::core::Buffer& buffer, std::tuple<Args...>& args)
    {
        size_t offset = 0;
        return DeserializeArgsRecursive(buffer, offset, args, std::index_sequence_for<Args...>{});
    }
    
    RpcMethodHash GetMethodHash(const std::string& method)
    {
        return std::hash<std::string>{}(method);
    }
private:
    template<typename Tuple, size_t... Index>
    bbt::core::errcode::ErrOpt DeserializeArgsRecursive(const bbt::core::Buffer& buffer, size_t& offset, Tuple& args, std::index_sequence<Index...>)
    {
        bbt::core::errcode::ErrOpt err;
        // 递归展开每个参数
        ((err = DeserializeOneArg(buffer, offset, std::get<Index>(args))) || ...);
        return err;
    }

    template<typename T>
    bbt::core::errcode::ErrOpt DeserializeOneArg(const bbt::core::Buffer& buffer, size_t& offset, T& arg)
    {
        FieldHeader header;

        // 检查类型是否匹配
        static_assert(IsSupportType<T>::value, "Unsupported type for deserialization!");
        if (!buffer.ToString(offset, (char*)&header, sizeof(header)))
            return bbt::core::errcode::Errcode("deserialize failed, buffer too short!", emErr::ERR_COMM);
        offset += sizeof(header);

        if (header.field_type != ToFieldType<T>())
            return bbt::core::errcode::Errcode("deserialize failed, field type mismatch! expected type=" + std::to_string(ToFieldType<T>()) + " , but it is actually=" + std::to_string(header.field_type), emErr::ERR_COMM);

        if constexpr (std::is_same_v<T, std::string>)
        {
            arg.resize(header.field_len);
            if (!buffer.ToString(offset, arg.data(), header.field_len))
                return bbt::core::errcode::Errcode("deserialize failed, buffer too short!", emErr::ERR_COMM);
        }
        else {
            if (header.field_len != sizeof(arg))
                return bbt::core::errcode::Errcode("deserialize failed, field length mismatch!", emErr::ERR_COMM);

            if (!buffer.ToString(offset, (char*)&arg, sizeof(arg)))
                return bbt::core::errcode::Errcode("deserialize failed, buffer too short!", emErr::ERR_COMM);
        }
        offset += header.field_len;

        return std::nullopt;
    }

    template<typename T>
    void SerializeArg(bbt::core::Buffer& bytes, T arg)
    {
        FieldHeader header;
        header.field_type = ToFieldType<T>();
        header.field_len = sizeof(arg);
        bytes.WriteString((char*)&header, sizeof(header));
        bytes.Write(arg);
    }

    void SerializeArg(bbt::core::Buffer& buffer, const std::string& arg)
    {
        FieldHeader header;
        header.field_type = STRING;
        header.field_len = arg.size();
        buffer.WriteString((char*)&header, sizeof(header));
        buffer.WriteString(arg.c_str(), arg.size());
    }

    void SerializeArg(bbt::core::Buffer& buffer, const char* arg)
    {
        FieldHeader header;
        header.field_type = STRING;
        header.field_len = strlen(arg);
        buffer.WriteString((char*)&header, sizeof(header));
        buffer.WriteString(arg, strlen(arg));
    }

    template<typename T, typename... Args>
    void SerializeArgs(bbt::core::Buffer& buffer, T arg, Args... args)
    {
        SerializeArg(buffer, arg);
        SerializeArgs(buffer, args...);
    }

    void SerializeArgs(bbt::core::Buffer& buffer) {}
};
} // namespace bbt::rpc::detail