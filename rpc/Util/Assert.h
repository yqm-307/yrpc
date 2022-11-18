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


///////////////////////////////////////////
///////////////////////////////////////////
//////////////     断言      //////////////
///////////////////////////////////////////
///////////////////////////////////////////


///////////////////////////////
//    断言信息
///////////////////////////////
#define Failed_Print_Info( cstr )\
    printf("YAssert Failed! | In file: %s | Line: %ld | Info: %s\n",__FILE__,__LINE__, cstr )\


// 运行时断言
#define YAssert_Failed(str) \
    {\
        auto info = str;\
        Failed_Print_Info( str.c_str() );\
        exit(-1);\
    }


#define YAssert( expr , fmt , ... )\
{\
    if (!(static_cast<bool>(expr)))\
    {\
        YAssert_Failed(yrpc::util::logger::format(fmt,##__VA_ARGS__));\
    }\
}





// 静态断言 编译期检测


/**
 * @brief 静态断言：类型 T，U 是否相同
 */
#define Static_Assert( expr , info )\
    {\
        static_assert(\
            static_cast<bool>(expr),\
            info);\
    }\



#define Static_Assert_Type_Same(T , U)\
    Static_Assert( yrpc::util::type::is_same_v<T,U> , "type same assert failed!" )

