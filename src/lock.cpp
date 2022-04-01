#include <include/lock.h>
#include <include/Logger.h>

using namespace udp;

mutex::mutex()
{
    int ret = 0;
    ret = pthread_mutex_init(&lock_,NULL);
    if(ret<0)
        FATAL("mutex::mutex() fail!  errno = %d",ret);
}

mutex::~mutex()
{
    int ret = 0;
    ret = pthread_mutex_destroy(&lock_);
    if(ret<0)
        FATAL("mutex::~mutex() fail!  errno = %d",ret);
}

int mutex::lock()
{   
    return pthread_mutex_lock(&lock_);
}

int mutex::unlock()
{
    return pthread_mutex_unlock(&lock_);
}

int mutex::trylock()
{
    return pthread_mutex_trylock(&lock_);
}

