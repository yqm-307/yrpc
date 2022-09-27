#pragma once
#include <functional>
#include <iostream>
#include <memory>
#include "../Util/Buffers.h"

namespace yrpc::detail::ynet
{

class Connection;
typedef std::shared_ptr<Connection> ConnectionPtr;
using std::placeholders::_1;
using std::placeholders::_2;
using std::placeholders::_3;
using std::placeholders::_4;
using std::placeholders::_5;

typedef std::function<void(const ConnectionPtr&,void*)> OnConnectHandle;
typedef std::function<void(const char*,size_t)> OnRecvHandle;
typedef std::function<void(const ConnectionPtr&)> ConnCloseHandle;
typedef std::function<void()> OnCloseHandle;


}