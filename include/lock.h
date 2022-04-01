#ifndef LOCK_H
#define LOCK_H

#include <pthread.h>
#include <lock_guard.h>
namespace udp
{
template<class LOCK_>
class LOCK_BASE
{
public:
    LOCK_BASE();
    virtual ~LOCK_BASE()=0;
    virtual int lock()=0;
    virtual int unlock()=0;
protected:
    LOCK_ lock_=0;
};

class mutex:public LOCK_BASE<pthread_mutex_t>
{
public:
    mutex();
    ~mutex();
    int lock();
    int unlock();
    virtual int trylock();
};
}
#endif