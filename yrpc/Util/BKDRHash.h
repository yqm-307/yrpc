#pragma once
#include <stdlib.h>
#include <iostream>
namespace yrpc::util
{

class hash
{
public:
    /**
     * @brief 根据字符串生成 hashid
     *
     * @param str 字符串
     * @param length 长度
     * @return uint32_t hash key
     */
    static uint32_t BKDRHash(const char *str, uint32_t length)
    {
        uint32_t seed = 1313; /* 31 131 1313 13131 131313 etc.. */
        uint32_t hash = 0;

        for (uint32_t i = 0; i < length; ++str, ++i)
        {
            hash = (hash * seed) + (*str);
        }

        return hash;
    }

    template<typename String>
    static uint32_t BKDRHash(String&& data)
    {
        uint32_t seed = 1313; /* 31 131 1313 13131 131313 etc.. */
        uint32_t hash = 0;

        for( auto&& c : data )
        {
            hash =  (hash * seed) + (c);
        }

        return hash;
    }
};
}