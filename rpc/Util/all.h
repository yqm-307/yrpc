/**
 * @file all.h
 * @author your name (you@domain.com)
 * @brief 
 * @version 0.1
 * @date 2022-06-05
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#pragma once
#include "logger.h"     // 日志
#include "BKDRHash.h"   // 字符串hash
#include "Buffers.h"    // bytearray/buffer
#include "Clock.h"      // clock 时钟日期工具
#include "Locker.h"     // 封装 pthread lock
#include "IDGenerate.h" // uid 生成器

#include "TimeQueue.h"  // 定时器队列
#include "TcpUtil.h"    // tcp util
#include "ThreadPool.h" // 通用线程池



#include "TypeList.h"   // 泛型工具
#include "noncopyable.h"    
#include "comparator.h" 

namespace yrpc::util    // 占位符
{

using std::placeholders::_1;
using std::placeholders::_2;
using std::placeholders::_3;
using std::placeholders::_4;
using std::placeholders::_5;

}