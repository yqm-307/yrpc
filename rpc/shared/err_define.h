#pragma once


namespace yrpc::detail::shared
{


enum YRPC_ERR_TYPE : int32_t
{
    ERR_TYPE_OK         = 0,
    ERRTYPE_NETWORK     = 100,


};


enum ERR_NETWORK : int32_t
{
    // err
    ERR_NETWORK_DEFAULT         = 0,    // 默认错误码
    ERR_NETWORK_SEND_FAIL       = 1001, // 发送失败
    ERR_NETWORK_RECV_FAIL       = 1002, // 接受数据失败
    ERR_NETWORK_CONN_CLOSED     = 1010, // 连接已关闭
    ERR_NETWORK_ECONNREFUSED    = 1111, // 连接被拒绝
    ERR_NETWORK_ACCEPT_FAIL     = 1121, // 接受连接失败

    // info
    ERR_NETWORK_SEND_OK         = 2002, // 发送成功
    ERR_NETWORK_RECV_OK         = 2003, // 接受成功
    ERR_NETWORK_CONN_OK         = 2004, // 连接建立成功
    ERR_NETWORK_CLOSE_OK        = 2005, // 连接关闭成功
    ERR_NETWORK_ACCEPT_OK       = 2006, // 接受连接成功
};

}