#pragma once
#include <type_traits>

namespace yrpc::detail::type
{

template<typename...>
struct List
{

};


template<typename T, typename... U>
struct List<T, U...>
{
    using Head = T;
    using Tail = List<U...>;
};

template<unsigned int, typename> struct TypeAt;


template<> struct TypeAt<0, List<>>
{
    using type = List<>;
};


template<typename Head, typename... Tail>
struct TypeAt<0, List<Head, Tail...>>
{
    using type = Head;
};


template<unsigned int idx, typename Head, typename... Tail>
struct TypeAt<idx, List<Head, Tail...>>
{
    using type = typename TypeAt<idx - 1, List<Tail...>>::type;
};




// // type检查
// template<class T,T v>
// struct intergral_constant
// {
//     static constexpr T value = v;
//     typedef T value_type;
//     typedef intergral_constant<T,v> self_type;

//     constexpr operator value_type() const noexcept {return value;}
//     constexpr value_type operator()() const noexcept {return value;}
// };



// typedef intergral_constant<bool,true> true_type;
// typedef intergral_constant<bool,false> false_type;

// template<class,class>
// struct is_same:public false_type{};

// template<class Type>
// struct is_same<Type,Type>:public true_type {};

template<class T,class U>
bool SameHelper = std::is_same_v<T,U>;

template<class T,class U>
bool same_as = SameHelper<T,U> && SameHelper<U,T>;


}