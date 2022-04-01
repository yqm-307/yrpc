#ifndef LOCK_GUARD_H
#define LOCK_GUARD_H
#include <pthread.h>

namespace udp
{
template<class _LOCK>
class lock_guard
{
public:
    lock_guard(_LOCK& lock)
        :lock_(lock)
    {
        lock_.lock();
    }
    ~lock_guard()
    {
        lock_.unlock();
    }
private:
    _LOCK& lock_;
};
}
#endif