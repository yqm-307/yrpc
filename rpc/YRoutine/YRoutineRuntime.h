#pragma once

#include <stdlib.h>

#include "rpc/YRoutine/YRoutineContext.h"   //协程上下文



namespace yrpc::coroutine
{

/**
 * @brief 协程运行时状态
 * 
 */
enum YRoutineRuntimeStatus
{
    RUNNING=0,
    BLOCK,
    SUSPEND
};



class YRoutineRuntime
{

};




}