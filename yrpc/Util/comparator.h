/**
 * @file comparator.h
 * @author yqm-307 (979336542@qq.com)
 * @brief 比较器接口类实现
 * @version 0.1
 * @date 2022-06-04
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#pragma once
#include <iostream>
#include <functional>

namespace yrpc::util
{

typedef std::function<bool(void*)> GreaterThenFunc;
typedef std::function<bool(void*)> EqualFunc;

/**
 * @brief 可以自动实现operator
 */
template<class Object>
class comparator
{
public:
    virtual bool operator==(const comparator<Object>& r_value_) const =0;
    virtual bool operator>(const comparator<Object>& r_value_) const = 0;
    

    
    virtual bool operator!=(const comparator<Object>& r_value_) const
    { return !(*this == r_value_); }
    

    virtual bool operator<(const comparator<Object>& r_value_) const
    { return (!this->operator>(r_value_)) && (this->operator!=(r_value_));}

    virtual bool operator>=(const comparator<Object>& r_value_) const 
    { return (this->operator>(r_value_) || (this->operator==(r_value_))); }

    virtual bool operator<=(const comparator<Object>& r_value_) const
    { return (this->operator<(r_value_) || (this->operator==(r_value_))); }

    virtual const Object& GetValue() const
    {
        return m_it;
    } 
    virtual Object& SetValue(const Object& obj)
    {
        return m_it=obj;
    }
protected:
    Object m_it;
};


}

