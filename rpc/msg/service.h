#pragma once
#include "./service_base.h"

class alltuple; //前置声明



namespace yrpc::detail
{

template<class Tuple>
class Service:public Service_base
{
public:
    typedef Tuple ArgsType;
    Service(){}
    ~Service(){}

protected:
};

}