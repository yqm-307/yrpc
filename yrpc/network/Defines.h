#pragma once
#include <functional>
#include <iostream>
#include <memory>
#include "../Util/Buffers.h"
#include "../Util/Assert.h"
#include "../shared/ErrCode.h"

// 收发超时
#define SOCKET_TIME_OUT_MS 3000

// 连接超时
#define SOCKET_CONN_TIME_MS 5000

namespace yrpc::detail::net
{

class Connection;
class YAddress;
typedef std::shared_ptr<Connection> ConnectionPtr;
typedef yrpc::detail::shared::errorcode errorcode;
using std::placeholders::_1;
using std::placeholders::_2;
using std::placeholders::_3;
using std::placeholders::_4;
using std::placeholders::_5;

typedef std::function<void(const errorcode&, const yrpc::detail::net::YAddress&, ConnectionPtr)>    OnConnectHandle;
typedef std::function<void(const errorcode&, ConnectionPtr)>    OnAcceptHandle;
typedef std::function<void(const errorcode&, yrpc::util::buffer::Buffer&)>       OnRecvHandle;
typedef std::function<void(const errorcode&, const ConnectionPtr&)>          ConnCloseHandle;
typedef std::function<void(const errorcode&)>                               OnCloseHandle;


}