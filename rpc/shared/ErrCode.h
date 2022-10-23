#pragma once
#include <string>
#include <cstring>
#include <stdarg.h>
#include "err_define.h"

namespace yrpc::detail::shared
{









class errorcode
{
public:
    template<class StrRef>
    errorcode(StrRef&& err_info,YRPC_ERR_TYPE errtype,int errcode)
        :m_err_info(err_info),
        m_err_type(errtype),
        m_errcode(errcode)
    {}

    errorcode()
        :m_err_info("nothing"),
        m_err_type(YRPC_ERR_TYPE::ERR_TYPE_OK),
        m_errcode(0)
    {}


    template<class StrRef>
    void setinfo(StrRef&& errinfo)
    { m_err_info = errinfo; }

    void setinfo(const char* fmt,...)
    {
        char data[128];
        size_t i = 0;
        va_list ap;

        va_start(ap, fmt);
        vsnprintf(data + i, 128 - i, fmt, ap);
        va_end(ap);
        m_err_info = data;
    }


    void setcode(int code)
    { m_errcode = code; }


    void settype(YRPC_ERR_TYPE type)
    { m_err_type = type; }

    template<class StrRef>
    void set(StrRef&& err_info,YRPC_ERR_TYPE errtype,int errcode)
    {
        m_err_info = err_info;
        m_err_type = errtype;
        m_errcode = errcode;
    }


    const std::string& what() const
    {
        return m_err_info;
    } 

    int err() const
    {
        return m_errcode;
    }


private:
    std::string     m_err_info;  
    YRPC_ERR_TYPE   m_err_type;
    int             m_errcode;

};


}