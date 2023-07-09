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
#include "../network/all.h"
#include "yrpc/shared/ErrCode.h"
#define CLIENTPORT

/* session 空闲存活时间，如果在session活跃时，session不会被释放。
但是如果超过 YRPC_SESSION_TIMEOUT 毫秒时间仍未有活跃，那么就会被
释放 
*/
#define YRPC_SESSION_TIMEOUT 3000

namespace yrpc::rpc::detail
{

typedef std::shared_ptr<google::protobuf::Message> MessagePtr;
typedef std::function<void(MessagePtr,const yrpc::detail::shared::errorcode&)> RpcCallback;
typedef std::function<void()>   CommCallback;

// rpc 调用返回状态
enum RPC_CALL_TYPE: int
{
    RPC_CALL_IS_WATING  = 0,
    RPC_CALL_IS_SUCCESS = 1,
    RPC_CALL_IS_TIMEOUT = 2,
    RPC_CALL_IS_FAILED  = 3,
    RPC_CALL_IS_SYNC    =4,
    
};


class RpcSession;
class __YRPC_SessionManager;


typedef __YRPC_SessionManager               SessionManager;
typedef uint64_t                            SessionID;
typedef yrpc::coroutine::poller::Epoller    Epoller;
typedef yrpc::coroutine::poller::RoutineSocket Socket;

typedef yrpc::util::lock::CountDownLatch    CountDownLatch;
typedef yrpc::util::lock::Mutex             Mutex;
typedef yrpc::util::buffer::Buffer          Buffer;

typedef std::shared_ptr<RpcSession>         SessionPtr;
typedef std::function<void(SessionPtr)>     OnSession;
typedef std::function<void()>               OnConnCallBack;

typedef yrpc::detail::net::Acceptor         Acceptor;
typedef yrpc::detail::net::YAddress         Address;
typedef yrpc::detail::net::ConnectionPtr    ConnPtr;
typedef yrpc::detail::net::Connector        Connector;
typedef yrpc::detail::net::errorcode        errorcode;
typedef yrpc::detail::net::ConnectionPtr    ConnectionPtr;

}