#pragma once
#include <bbt/rpc/detail/Define.hpp>

namespace bbt::rpc::codec
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
    INT16,
    UINT16,
    INT8,
    UINT8,
    STRING,
    POD, // plain old data
    BYTEOBJ,
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
inline FieldType ToFieldType();

/**
 * @brief 将args打包为字节流并返回
 * 
 * @tparam Args 
 * @param args 
 * @return bbt::core::Buffer 
 */
template<typename ...Args>
inline bbt::core::Buffer Serialize(Args... args);

template<typename Tuple>
inline bbt::core::Buffer SerializeWithTuple(Tuple&& tuple);

template<typename ...Args>
inline void SerializeAppend(bbt::core::Buffer& buffer, Args... args);

template<typename Tuple>
inline void SerializeAppendWithTuple(bbt::core::Buffer& buffer, Tuple&& args);

template<typename... Args>
inline bbt::core::errcode::ErrOpt DeserializeWithTuple(const bbt::core::Buffer& buffer, std::tuple<Args...>& args);

inline RpcMethodHash GetMethodHash(const std::string& method)
{
    return std::hash<std::string>{}(method);
}
template<typename Tuple, size_t... Index>
inline bbt::core::errcode::ErrOpt DeserializeArgsRecursive(const bbt::core::Buffer& buffer, size_t& offset, Tuple& args, std::index_sequence<Index...>);

template<typename T>
inline bbt::core::errcode::ErrOpt DeserializeOneArg(const bbt::core::Buffer& buffer, size_t& offset, T& arg);

template<typename T>
inline void SerializeArg(bbt::core::Buffer& bytes, T arg);

inline void SerializeArg(bbt::core::Buffer& buffer, const std::string& arg);

inline void SerializeArg(bbt::core::Buffer& buffer, const char* arg);

template<typename T, typename... Args>
inline void SerializeArgs(bbt::core::Buffer& buffer, T arg, Args... args);

} // namespace bbt::rpc::detail

#include <bbt/rpc/detail/__TRpcCodec.hpp>