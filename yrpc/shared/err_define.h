#pragma once


namespace yrpc::detail::shared
{


enum YRPC_ERR_TYPE : int32_t
{
    ERRTYPE_NOTHING     = 0,
    ERRTYPE_NETWORK     = 100,  // 网络io错误
    ERRTYPE_HANDSHAKE   = 101,  // Session握手
};

/* 网络错误码 */
enum ERR_NETWORK : int32_t
{
    // err
    ERR_NETWORK_DEFAULT         = 0,    // 默认错误码
    ERR_NETWORK_SEND_FAIL       = 1001, // 发送失败
    ERR_NETWORK_RECV_FAIL       = 1002, // 接受数据失败
    ERR_NETWORK_CONN_CLOSED     = 1010, // 连接已关闭
    ERR_NETWORK_CONN_OTHRE_ERR  = 1011, // 连接建立失败，返回errno
    ERR_NETWORK_ECONNREFUSED    = 1111, // 连接被拒绝
    ERR_NETWORK_ACCEPT_FAIL     = 1121, // 接受连接失败

    // info
    ERR_NETWORK_SEND_OK         = 2002, // 发送成功
    ERR_NETWORK_RECV_OK         = 2003, // 接受成功
    ERR_NETWORK_CONN_OK         = 2004, // 连接建立成功
    ERR_NETWORK_CLOSE_OK        = 2005, // 连接关闭成功
    ERR_NETWORK_ACCEPT_OK       = 2006, // 接受连接成功
};

enum ERR_HANDSHAKE : int32_t
{
    ERR_HANDSHAKE_SUCCESS       = 0,    // 握手成功完成
    ERR_HANDSHAKE_TIMEOUT       = 1001, // 握手超时
    ERR_HANDSHAKE_SESS_NOTEXIST = 1002, // Session 找不到
    ERR_HANDSHAKE_UNDONE_FAILED = 1003, // 半连接队列错误
};

}