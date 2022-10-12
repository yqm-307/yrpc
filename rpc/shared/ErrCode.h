#pragma once
#include <string>
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