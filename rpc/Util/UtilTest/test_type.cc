#include "../TypeList.h"
#include <iostream>

using namespace yrpc::detail::type;
using namespace std;


int main()
{
    if(yrpc::detail::type::same_as<int,bool>)
    {
        printf("bad\n");
    }

    if(yrpc::detail::type::same_as<int,int>)
    {
        printf("good\n");
    }
}
