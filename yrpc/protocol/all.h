/**
 * @file all.h
 * @author your name (you@domain.com)
 * @brief 
 * @version 0.1
 * @date 2022-09-23
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#pragma once

#include "Codec.h"          // protobuf 解析/序列化
#include "Define.h"         // 定义常用枚举和typedef
#include "proto.h"          // protobuf 工具
#include "YProtocolGenerater.h"     // rpc请求封装
#include "YProtocolResolver.h"    // rpc响应封装



// #include "Protocol.h"       //  v1.0版本设计，欲弃用 (已删除)
#include "ProtocolUtil.h"   //  v1.0版本设计，欲弃用
#include "ErrCode.h"        //  v1.0版本设计，欲弃用