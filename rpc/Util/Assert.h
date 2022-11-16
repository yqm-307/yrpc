/**
 * @file Assert.h
 * @author your name (you@domain.com)
 * @brief 
 * @version 0.1
 * @date 2022-11-16
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#pragma once
#include "logger.h"
#include <assert.h>
#include <iostream>
#include <typeinfo>
#include <type_traits>
#include "Type.h"


// 运行时断言
#define YAssert_Failed(str) \
    {\
        auto info = str;\
        printf("YAssert Failed! | In file: %s | Line: %ld | Info: %s\n",__FILE__,__LINE__,info.c_str() );\
        exit(-1);\
    }


#define YAssert( expr , fmt , ... )\
{\
    if (!(expr))\
    {\
        YAssert_Failed(yrpc::util::logger::format(fmt,##__VA_ARGS__));\
    }\
}





// 静态断言 编译期检测

#define Static_Assert_Type_Same(T , U) static_assert(yrpc::util::type::is_same_v<T,U>)