#pragma once
#include <type_traits>

namespace yrpc::util::type
{



template<bool,class U>
struct TypeIs_Detail
{};


template<typename U>
struct TypeIs_Detail<true,U>
{ typedef U type; };


/**
 * @brief 如果 模板参数T 和 模板参数U 相同，则推断出is_same_v 为 true
 *  偏特化成功，最终TypeIs推断为U
 * 
 * @tparam T 模板参数
 * @tparam U 被匹配的模板参数,匹配成功,TypeIs为U;匹配失败,编译不通过
 */
template<class T,class U>
using TypeIs = typename TypeIs_Detail<std::is_same_v<T,U>,U>::type;





}