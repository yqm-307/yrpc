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

#pragma pack(push, 1)

struct FieldHeader
{
    int8_t field_type;
    int16_t field_len;
};

struct FieldValue
{
    FieldHeader header;
    union RpcSerializerValue
    {
        int32_t int32_value;
        int64_t uint32_value;
        int64_t int64_value;
        uint64_t uint64_value;
    } value;
    std::string string;
};

#pragma pack(pop)

//TODO 先支持最简单的类型，后续拓展
enum FieldType : int8_t
{
    INT64 = 1,
    UINT64,
    INT32,
    UINT32,
    STRING,
};


template<typename T> struct IsSupportType { static constexpr bool value = false; };

template<> struct IsSupportType<int32_t> { static constexpr bool value = true; };

template<> struct IsSupportType<uint32_t> { static constexpr bool value = true; };

template<> struct IsSupportType<int64_t> { static constexpr bool value = true; };

template<> struct IsSupportType<uint64_t> { static constexpr bool value = true; };

template<> struct IsSupportType<std::string> { static constexpr bool value = true; };

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
        static_assert(IsSupportType<T>::value, "Unsupported type for ToFieldType");
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

    template<typename ...Args>
    void SerializeAppend(bbt::core::Buffer& buffer, Args... args)
    {
        SerializeArgs(buffer, args...);
    }

    template<typename... Args>
    bbt::core::errcode::ErrOpt DeserializeWithArgs(const bbt::core::Buffer& buffer, std::tuple<Args...>& args)
    {
        size_t offset = 0;
        return DeserializeArgsRecursive(buffer, offset, args, std::index_sequence_for<Args...>{});
    }

    bbt::core::errcode::ErrTuple<std::vector<FieldValue>> Deserialize(const bbt::core::Buffer& buffer)
    {
        std::vector<FieldValue> values;
        FieldValue value;
        size_t offset = 0;

        while (buffer.Size() > offset)
        {
            if (buffer.Size() < sizeof(value.header))
                return {bbt::core::errcode::Errcode{"deserialize failed, buffer too short!", emErr::ERR_COMM}, values};

            auto err = DeserializeOne(buffer, offset, value);
            if (err != std::nullopt)
                return {err, values};
            
            values.push_back(value);
        }

        return {std::nullopt, values};
    }

    bbt::core::errcode::ErrOpt DeserializeOne(const bbt::core::Buffer& buffer, size_t& offset, FieldValue& value)
    {
        Assert(buffer.ToString(offset, (char*)&(value.header), sizeof(value.header)));

        switch (value.header.field_type)
        {
        case INT64:
        case UINT64:
            Assert(buffer.ToString(offset + sizeof(value.header), (char*)&(value.value), sizeof(value.value)));
            offset += sizeof(value.header) + sizeof(value.value.int64_value);
            break;
        case INT32:
        case UINT32:
            Assert(buffer.ToString(offset + sizeof(value.header), (char*)&(value.value.int32_value), sizeof(value.value.int32_value)));
            offset += sizeof(value.header) + sizeof(value.value.int32_value);
            break;
        case STRING:
            value.string.resize(value.header.field_len);
            Assert(buffer.ToString(offset + sizeof(value.header), value.string.data(), value.string.size()));
            offset += sizeof(value.header) + value.string.size();
            break;
        default:
            return bbt::core::errcode::Errcode("deserialize failed, not support this type!", emErr::ERR_COMM);
        }

        return std::nullopt;
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
        FieldValue value;
        auto err = DeserializeOne(buffer, offset, value);
        if (err != std::nullopt)
            return err;

        // 检查类型是否匹配
        if (value.header.field_type != ToFieldType<T>())
            return bbt::core::errcode::Errcode("deserialize failed, type mismatch!", emErr::ERR_COMM);

        // 根据类型赋值
        if constexpr (std::is_same_v<T, std::string>)
        {
            arg = value.string;
        }
        else if constexpr (std::is_trivial_v<T>)
        {
            arg = static_cast<T>(value.value.uint64_value);
        }
        else
        {
            return bbt::core::errcode::Errcode("deserialize failed, unsupported type!", emErr::ERR_COMM);
        }

        return std::nullopt;
    }

    template<typename T>
    void SerializeArg(bbt::core::Buffer& bytes, T arg)
    {
        AssertWithInfo(false, "Not support this type");
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

    void SerializeArg(bbt::core::Buffer& buffer, int64_t arg)
    {
        FieldHeader header;
        header.field_type = INT64;
        header.field_len = sizeof(arg);
        buffer.WriteString((char*)&header, sizeof(header));
        buffer.Write(arg);
    }

    void SerializeArg(bbt::core::Buffer& buffer, uint64_t arg)
    {
        FieldHeader header;
        header.field_type = UINT64;
        header.field_len = sizeof(arg);
        buffer.WriteString((char*)&header, sizeof(header));
        buffer.Write(arg);
    }

    void SerializeArg(bbt::core::Buffer& buffer, int32_t arg)
    {
        FieldHeader header;
        header.field_type = INT32;
        header.field_len = sizeof(arg);
        buffer.WriteString((char*)&header, sizeof(header));
        buffer.Write(arg);
    }

    void SerializeArg(bbt::core::Buffer& buffer, uint32_t arg)
    {
        FieldHeader header;
        header.field_type = UINT32;
        header.field_len = sizeof(arg);
        buffer.WriteString((char*)&header, sizeof(header));
        buffer.Write(arg);
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