#pragma once
#include <stdint.h>

namespace yrpc::detail::protocol::def
{
/**
 * @brief yrpc 使用的协议集合
 *
 * 在 YRPC 中，使用到的所有协议类型，都会在下面定义，而且协议类型包含在 protocol head 里面。
 * 就是说所有 YRPC 内部（用户自定义 protocol buffer 不算内部协议，因为进入YRPC已经序列化为
 * 字节流，或者说都算是 RPC_CALL_RSP 或者 RPC_CALL_REQ）使用的协议，都在下面的枚举值中。
 * 
 * 如果需要判断详细的req、rsp类型，就需要 google protobuf 相关api支持
 * 
 * 约定大于30000是 服务端响应
 */
enum YRPC_PROTOCOL : uint16_t
{
    type_YRPC_PROTOCOL_Done = 0,

    type_C2S_HEARTBEAT_REQ = 10000,
    type_C2S_RPC_CALL_REQ = 10010,

    /* 用来区分 c2s 和 s2c 协议的分界线*/
    /* 小于30000的都是请求,大于30000的都是响应 */ 
#define type_YRPC_PROTOCOL_CS_LIMIT 30000   
    type_S2C_HEARTBEAT_RSP = 30001,
    type_S2C_RPC_CALL_RSP = 30011,
    type_S2C_RPC_ERROR = 30012,         
};
}