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
    ERR_NETWORK_DEFAULT         = 0,
    ERR_NETWORK_SEND_FAIL       = 1001,
    ERR_NETWORK_ECONNREFUSED    = 1111,

    // info
    ERR_NETWORK_SEND_OK         = 2002,
    ERR_NETWORK_RECV_OK         = 2003,
    ERR_NETWORK_CONN_OK         = 2004,
    
};

}