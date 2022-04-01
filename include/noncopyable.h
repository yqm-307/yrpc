#ifndef NONCOPYABLE_H
#define NONCOPYABLE_H

namespace udp
{
//子类不可以被拷贝
class noncopyable
{
public:
    noncopyable(noncopyable&)=delete;
    noncopyable& operator=(noncopyable&)=delete;
protected:
    noncopyable()=default;
    ~noncopyable()=default;
};
}
#endif