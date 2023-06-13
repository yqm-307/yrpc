#include "../Type.h"
#include <iostream>

using namespace yrpc::util::type;
using namespace std;


template<class T,typename = TypeIs<T,int>>
int func1(T val)
{
    cout<< "int" <<endl;
}

int main()
{
    func1(1);
    // func1(2.1); //编译不通过，模板参数推导失败
    

}
