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


}