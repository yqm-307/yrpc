#pragma once
#include <functional>
#include <iostream>
#include <memory>
#include "../Util/Buffers.h"
#include "../shared/ErrCode.h"

namespace yrpc::detail::net
{

class Connection;
typedef std::shared_ptr<Connection> ConnectionPtr;
typedef yrpc::detail::shared::errorcode errorcode;
using std::placeholders::_1;
using std::placeholders::_2;
using std::placeholders::_3;
using std::placeholders::_4;
using std::placeholders::_5;

typedef std::function<void(const errorcode&,ConnectionPtr,void*)>    OnConnectHandle;
typedef std::function<void(const errorcode&,const char*,size_t)>            OnRecvHandle;
typedef std::function<void(const errorcode&,const ConnectionPtr&)>          ConnCloseHandle;
typedef std::function<void(const errorcode&)>                               OnCloseHandle;


}