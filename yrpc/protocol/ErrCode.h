#pragma once
// #include "../Util/BKDRHash.h"
#include "Define.h"
namespace yrpc::err
{
/* 整合到define中 */
// //错误报文和请求
// enum YRPC_CALL_ERROR_CODE
// {
//     CALL_SUCCESS=0,
//     CALL_FATAL_SERVICE_ID_IS_BAD,
//     CALL_FATAL_SERVICE_MSG_IS_BAD,
//     CALL_FATAL_SERVICE_TIMEOUT,
//     CALL_FATAL_SERVER_BUSY
// };


// enum YRPC_ErrCode
// {
//     // future完成状态
//     DONE = 0,               // 未知错误
//     FUTURE_CALL_SUCCESS=1,  // 调用正常完成
//     FUTURE_CALL_FATAL,      // 调用失败
//     FUTURE_IN_A_CALL        // 正在执行一次调用
// };


class errcode
{
    typedef yrpc::detail::protocol::define::YRPC_ErrCode YRPC_ErrCode;
public:
    errcode(YRPC_ErrCode err = yrpc::detail::protocol::define::CALL_FATAL_OTHRE,std::string info="")
        :errinfo(info),
        code(err)
    {}

    std::string what() const 
    { return errinfo; }

    bool operator==(YRPC_ErrCode err)const 
    { return (err == code); }

    void SetErrInfo(const std::string &info)
    { errinfo = info; }
    void SetErrCode(YRPC_ErrCode err)
    { code = err; }

private:
    std::string errinfo;
    YRPC_ErrCode code;
};

// class ERRUTIL
// {
// public:
//     static bool IsMsgErrCode(const char *str, size_t len);
//     static int GetMsgErrCode(const char *str, size_t len);
// };


// extern bool IsMsgErrCode(const char *str, size_t len);
// extern int GetMsgErrCode(const char *str, size_t len);
}
