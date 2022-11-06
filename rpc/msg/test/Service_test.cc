

#include "../servicemap.h"
#include "../../protocol/proto.h"
#include "../../protocol/Codec.h"
#include "../../proto/test_protocol/addr.pb.h"
using namespace yrpc::detail;

#define tuple_begin class alltuple\
{\
    public:
#define tuple_type(x,index) typedef decltype(x)::ArgsType Type_##index 

#define tuple_end }

// tuple_begin
// Service<std::tuple<int,int>> add;
// tuple_type(add,0);
// Service<std::tuple<std::string>> str; 
// tuple_type(str,1);
// tuple_end;



/* 
    Codec 测试案例
*/
using namespace google::protobuf;
void test1()
{
    yrpc::detail::Codec codec;  //编码解码器

    address addr;
    addr.set_a(INT_MAX);
    addr.set_b(true);
    addr.set_c("hello world!");
    addr.set_d(INT64_MAX);


    std::string bytes;
    codec.Serialize<address>(&addr,bytes);


    Message* msg = new address;
    //解析程序，返回包含所有数据的vector
    auto ptr = codec.Parse<address>(bytes);
    
    
    int res1 = std::any_cast<int>((*ptr)[0]);
    printf("a: %d\n",res1);
    bool res2 = std::any_cast<bool>((*ptr)[1]);
    printf("b: %d\n",res2);
    std::string res3 = std::any_cast<std::string>((*ptr)[2]);
    printf("c: %s\n",res3.c_str());
    int64 res4 = std::any_cast<int64>((*ptr)[3]);
    printf("d: %ld\n",res4);

}

#define DD(type) [](bool is_parse,std::any& arg1,std::any& arg2){\
if(is_parse)\
    std::any_cast<ArgsPtr&>(arg2) = Codec::Parse<type>(std::any_cast<std::string_view&>(arg1));\
else\
    Codec::Serialize<type>(std::any_cast<type*>(arg1),std::any_cast<std::string&>(arg2));\
}

void test2()
{
    ServiceMap::GetInstance()->insert("add",
    [](std::any args){
        auto ptr = std::any_cast<std::shared_ptr<addreq>>(args);
        auto req = (addreq*)(ptr.get());

        int a = req->rint();
        int b = req->lint();
        auto rsp = new addrsp();
        rsp->set_result(a+b);
        printf("调用成功: %d + %d = %d\n",a,b,a+b);
        return rsp;   //返回值直接send
    },DD(addreq));    
    //服务注册完毕
    printf("服务注册完毕\n");
}

void calltest1(int a,int b)
{
    addreq req;
    req.set_rint(a);
    req.set_lint(b);
    std::string beforebytes;
    Codec::Serialize<addreq>(&req,beforebytes);

    
    std::any args;
    std::any bytes_ = beforebytes; 

    ServiceMap::GetInstance()->NameToService("add")->second(true,bytes_,args);

    auto msg = std::any_cast<std::shared_ptr<addrsp>>(args);

    auto str = ServiceMap::GetInstance()->NameToService("add")->first(msg);

}



//测试注册到map中是否可行
int main(int args,char** argv)
{
    if(args <3)
    {
        printf("参数少于2\n");
        exit(-1);
    }


    //test1();
    test2();
    calltest1(atoi(argv[1]),atoi(argv[2]));
    //calltest1(1,2);
}