#pragma once


namespace yrpc::detail::shared
{


enum YRPC_ERR_TYPE
{
    ERR_TYPE_OK         = 0,
    ERRTYPE_NETWORK     = 100,


};


enum ERR_NETWORK
{
    // err
    ERR_NETWORK_DEFAULT     = 0,
    ERR_NETWORK_SEND_FAIL   = 1001,

    // info
    ERR_NETWORK_SEND_OK     = 2002,
    ERR_NETWORK_RECV_OK     = 2003,
    
};

}