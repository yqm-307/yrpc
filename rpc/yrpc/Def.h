#pragma once
namespace yrpc::rpc::def
{
// rpc 调用返回状态
enum RPC_CALL_TYPE: int
{
    RPC_CALL_IS_WATING  = 0,
    RPC_CALL_IS_SUCCESS = 1,
    RPC_CALL_IS_TIMEOUT = 2,
    RPC_CALL_IS_FAILED  = 3,
    RPC_CALL_IS_SYNC    =4,
    
};
}