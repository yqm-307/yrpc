/**
 * @file Define.h
 * @author your name (you@domain.com)
 * @brief 宏
 * @version 0.1
 * @date 2022-10-06
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#pragma once
#include <functional>
#include <memory>
#include <google/protobuf/any.h>
#include "../protocol/ErrCode.h"
#define CLIENTPORT


namespace yrpc::rpc::detail
{

typedef std::shared_ptr<google::protobuf::Message> MessagePtr;
typedef std::function<void(MessagePtr,const yrpc::err::errcode&)> RpcCallback;

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