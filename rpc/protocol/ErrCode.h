#pragma once
#include "../Util/BKDRHash.h"
namespace yrpc::err
{

//错误报文和请求
enum YRPC_CALL_ERROR_CODE
{
    CALL_SUCCESS=0,
    CALL_FATAL_SERVICE_ID_IS_BAD,
    CALL_FATAL_SERVICE_MSG_IS_BAD,
    CALL_FATAL_SERVICE_TIMEOUT,
    CALL_FATAL_SERVER_BUSY
};


enum YRPC_ERRCODE
{
    // future完成状态
    FUTURE_CALL_SUCCESS=0,  //调用正常完成
    FUTURE_CALL_FATAL,      //调用失败
    FUTURE_IN_A_CALL        //正在执行一次调用

};

// class ERRUTIL
// {
// public:
//     static bool IsMsgErrCode(const char *str, size_t len);
//     static int GetMsgErrCode(const char *str, size_t len);
// };


extern bool IsMsgErrCode(const char *str, size_t len);
extern int GetMsgErrCode(const char *str, size_t len);
}
