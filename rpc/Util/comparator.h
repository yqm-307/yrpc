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
template<class Item>
class comparator
{
public:
    virtual bool operator==(const comparator<Item>& r_value_) const =0;
    virtual bool operator>(const comparator<Item>& r_value_) const = 0;
    

    
    virtual bool operator!=(const comparator<Item>& r_value_) const
    { return !(*this == r_value_); }
    

    virtual bool operator<(const comparator<Item>& r_value_) const
    { return (!this->operator>(r_value_)) && (this->operator!=(r_value_));}

    virtual bool operator>=(const comparator<Item>& r_value_) const 
    { return (this->operator>(r_value_) || (this->operator==(r_value_))); }

    virtual bool operator<=(const comparator<Item>& r_value_) const
    { return (this->operator<(r_value_) || (this->operator==(r_value_))); }

    virtual const Item& GetValue() const
    {
        return it_;
    } 
    virtual Item& SetValue(const Item& obj)
    {
        return it_=obj;
    }
protected:
    Item it_;
};


}

