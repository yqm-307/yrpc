/**
 * @file noncopyable.h
 * @author yqm-307 (979336542@qq.com)
 * @brief noncopyable类的实现直接从libyqmnet引用来
 * @version 0.1
 * @date 2022-06-04
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#pragma once

namespace yrpc::util::noncopyable
{

class noncopyable
{
public:
    noncopyable(noncopyable& non) = delete;
    void operator=(noncopyable& non) = delete;

protected:
    noncopyable() = default;
    ~noncopyable() = default;
};

}





