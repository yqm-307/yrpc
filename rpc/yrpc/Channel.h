#pragma once
#include "../network/all.h"

namespace yrpc::rpc::detail
{


// io 通道，包含tcp connection
class Channel
{
public:
    Channel();
    ~Channel();

    void SetRecvCallback()
    {}

private:
    yrpc::detail::net::Connection       m_conn;     // 连接

};



}