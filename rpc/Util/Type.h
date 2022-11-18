#pragma once
#include <type_traits>

namespace yrpc::util::type
{

#if (__cpluscplus >= 201103L && __cpluscplus < 201703L)
    template<typename T ,typename  U>
    inline constexpr bool is_same_v = std::is_same<T,U>::value;
#elif ( __cplusplus >= 201703L )
    template<typename T, typename U>
    inline constexpr bool is_same_v = std::is_same_v<T,U>;
#endif


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
using TypeIs = typename TypeIs_Detail<is_same_v<T,U>,U>::type;

/**
 *  如果 类型参数T 等于 类型参数U ，则可以成功推断出模板类型T
 *  否则，推断失败，无法通过编译。
 * 
 *  使用此声明 if_same_as(T,U)，可以强制让T等于U才能通过编译
 */
#define if_same_as( T , U ) \
    typename = yrpc::util::type::TypeIs<T,U>\


/**
 *  为了支持静态类型推断
 *  lambda 强转 function，性能没问题，将lambda作为参数传递，接收方为万能引用即可
 *  function支持移动语义
 */
#define functor( lambda ) std::function( lambda )



}