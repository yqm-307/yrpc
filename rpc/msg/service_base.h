#pragma once
#include <functional>
#include <iostream>
#include <tuple>
#include "../network/Connection.h"
#include "../protocol/Codec.h"


namespace yrpc::detail
{




/**
 * @brief 服务对象，用户需要注册服务对象，才能对外提供可以访问的服务
 * 
 */
class Service_base
{
public:
    Service_base(){}
    virtual ~Service_base(){}

    virtual void run()=0;

    template<class Tuple>
    void setArgs(Tuple& args)
    {
        getArgs<Tuple>() = args;
    }

    template<class Tuple>
    Tuple& getArgs()
    {
        static Tuple Args;
        return Args;
    }
};



}
