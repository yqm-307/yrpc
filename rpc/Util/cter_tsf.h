/**
 * @file map_tsf.h
 * @author your name (you@domain.com)
 * @brief 线程安全的 std::map、std::set、std::unordered_map、std::unordered_set
 * @version 0.1
 * @date 2022-09-23
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#include <map>
#include <set>
#include <unordered_map>
#include <unordered_set>
#include "../Util/Locker.h"

#ifndef LOCAL_CTER_TSF_H
#define CRITICAL_BEGIN( mutex )   \
{\
    yrpc::util::lock::lock_guard<yrpc::util::lock::Mutex> lock( mutex );
#define CRITICAL_END }

#define EXISTIN_MAP( map , key , it) (((it = map.find( key )) == map.end()) ? false : true)



namespace yrpc::util::thread_safe_cter
{



template<class K,class V,class Base_Cter>
class thread_saft_cter
{};



/**
 * @brief std::map<k,v> , but is thread safe
 */
template<class K, class V>   //base container
class thread_saft_cter<K,V,std::map<K,V>>
{
public:
    typedef typename std::map<K,V>::iterator     _Iter;
    typedef typename std::map<K,V>::key_type     _TpKey;
    typedef typename std::map<K,V>::value_type   _TpValue;
    typedef typename std::pair<K,V>              _TpPair;
    typedef typename std::pair<_Iter,bool>       _reture_type;

public:
    
    thread_saft_cter(){}
    ~thread_saft_cter(){}


    /**
     * @brief 从map中获取值并删除键值对
     * 
     * @param key 键
     * @param value(out) 值 
     * @return true 成功
     * @return false 失败,找不到
     */
    bool GetValueAndRemove(const _TpKey& key,_TpValue& value)
    {
        bool is_exist = false;
        CRITICAL_BEGIN( m_mutex )
        _Iter it;
        
        if( EXISTIN_MAP( m_Cter , key , it) )
        {    
            value = it->second;
            it = m_Cter.erase();
            return true;
        }
        CRITICAL_END

        return false;
    }    

    /**
     * @brief 从map中删除键为 key 的键值对
     * 
     * @param key 键值
     * @return true 删除成功
     * @return false 删除失败
     */
    bool Remove(const _TpKey& key)
    {
        CRITICAL_BEGIN( m_mutex )

        _Iter it;
        if( EXISTIN_MAP( m_Cter , key , it ) )
        {
            m_Cter.erase(it);
            return true;
        }
        CRITICAL_END
        return false;
    }


    bool Insert(const _TpPair& pair)
    {
        CRITICAL_BEGIN( m_mutex )
        _Iter it;
        if ( ! EXISTIN_MAP( m_Cter , pair->first , it ) )
        {
            return m_Cter.insert(std::move(pair));
        }
        CRITICAL_END
        return m_Cter.end();
    }

    /**
     * @brief 插入并返回迭代器，失败返回end
     * 
     * @param pair 插入的键值对
     * @return _Iter 
     */
    _Iter Insert(const _TpPair&& pair)
    {
        CRITICAL_BEGIN( m_mutex )
        _Iter it;
        if ( ! EXISTIN_MAP( m_Cter , pair.first , it ) )
        {
            return m_Cter.insert(pair).first;
        }
        CRITICAL_END
        return m_Cter.end();
    }


    _Iter FindKey(const _TpKey& key)
    {
        CRITICAL_BEGIN( m_mutex )

        _Iter it;
        if( ! EXISTIN_MAP( m_Cter , key , it ) )
        {
            return it;
        }

        CRITICAL_END
        return m_Cter.end();
    }

    bool IsEnd(const _Iter& it)
    {
        return it == m_Cter.end();
    }
    
    _Iter FindKey(const _TpKey key)
    {
        CRITICAL_BEGIN( m_mutex )

        _Iter it;
        if( ! EXISTIN_MAP( m_Cter , key , it ) )
        {
            return it;
        }

        CRITICAL_END
        return m_Cter.end();
    }

    size_t Size()
    {
        CRITICAL_BEGIN(m_mutex)
        return m_Cter.size();
        CRITICAL_END
    }


protected:
    std::map<K,V> m_Cter;
    yrpc::util::lock::Mutex m_mutex;
};






template<class V,class Base_Cter>
class thread_saft_cter_o
{};


template<class V>
class thread_saft_cter_o<V,std::queue<V>>
{   
public:
    typedef V _TpValue;
public:
    thread_saft_cter_o(){}
    ~thread_saft_cter_o(){}

    void Push(const _TpValue& val)
    {   
        CRITICAL_BEGIN( m_mutex )
        m_queue.push(std::move(val));
        CRITICAL_END
    }

    _TpValue Pop()
    {
        CRITICAL_BEGIN( m_mutex )
        auto ret = m_queue.front();
        m_queue.pop();
        return ret;
        CRITICAL_END
    } 

    bool Empty()
    {
        CRITICAL_BEGIN( m_mutex )
        return m_queue.empty();
        CRITICAL_END
    }

    size_t Size()
    {
        CRITICAL_BEGIN( m_mutex )
        return m_queue.size();
        CRITICAL_END
    }




protected:
    std::queue<V> m_queue;
    yrpc::util::lock::Mutex m_mutex;
};

}



#undef CRITICAL_BEGIN

#undef CRITICAL_END

#undef EXISTIN_MAP

#endif




//  could not convert ‘((yrpc::util::thread_safe_cter::thread_saft_cter<int, int, std::map<int, int> >*)this)->yrpc::util::thread_safe_cter::thread_saft_cter<int, int, std::map<int, int> >::m_Cter.std::map<int, int>::insert<const std::pair<int, int>&>((* & pair))’ from 
 
//  ‘std::__enable_if_t<true, 
//  std::pair<std::_Rb_tree_iterator<std::pair<const int, int> >, bool> >’
//   {aka ‘std::pair<std::_Rb_tree_iterator<std::pair<const int, int> >, bool>’} to 
//   ‘yrpc::util::thread_safe_cter::thread_saft_cter<int, int, std::map<int, int> >::_Iter’ {aka ‘std::_Rb_tree_iterator<std::pair<const int, int> >’}