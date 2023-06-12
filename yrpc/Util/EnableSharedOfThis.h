#pragma once
#include <memory>

namespace yrpc::util::enable_shared_from_this
{

template<class Type>
class enable_shared_from_this
{
public:

    std::shared_ptr<Type> shared_from_this()
    { return std::shared_ptr<Type>(weak_this_ptr); }
    std::shared_ptr<const Type> shared_from_this() const
    { return std::shared_ptr<const Type>(weak_this_ptr); }

    template<class Up> friend class shared_ptr;
protected:
    enable_shared_from_this() {}
    enable_shared_from_this(const enable_shared_from_this& ){}
    enable_shared_from_this& operator=(const enable_shared_from_this&)
    { return *this; }
    ~enable_shared_from_this() {}


private:
    mutable std::weak_ptr<Type> weak_this_ptr;
};

}