#include <string.h>
#include <iostream>
#include "ErrCode.h"

using namespace yrpc::err;

// bool yrpc::err::ERRUTIL::IsMsgErrCode(const char *str, size_t len)
// {
//     if (len < sizeof(uint32_t))
//     {
//         return false;
//     }
//     else
//     {
//         uint32_t p;
//         memcpy((char *)(&p), str, sizeof(uint32_t));
//         if(p == yrpc::util::BKDRHash("CALLERR",sizeof("CALLERR")))
//             return true;
//     }
// }


// int yrpc::err::ERRUTIL::GetMsgErrCode(const char *str, size_t len)
// {
//     if (len < sizeof(uint32_t))
//         return -1;
//     else
//     {
//         int err = -2;
//         const char *p = strchr(str, '#');
//         if (p == NULL)
//             return -3;
//         err = atoi((p + 1));
//     }
// }
bool yrpc::err::IsMsgErrCode(const char *str, size_t len)
{
    if (len < sizeof(uint32_t))
    {
        return false;
    }
    else
    {
        uint32_t p;
        memcpy((char *)(&p), str, sizeof(uint32_t));
        if(p == yrpc::util::hash::BKDRHash("CALLERR",sizeof("CALLERR")))
            return true;
    }
}


int yrpc::err::GetMsgErrCode(const char *str, size_t len)
{
    if (len < sizeof(uint32_t))
        return -1;
    else
    {
        int err = -2;
        const char *p = strchr(str, '#');
        if (p == NULL)
            return -3;
        err = atoi((p + 1));
    }
}