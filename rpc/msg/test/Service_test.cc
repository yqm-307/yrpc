#include "../servicemap.h"
#include "../../protocol/proto.h"
#include "../../protocol/Codec.h"
#include "../../proto/test_protocol/addr.pb.h"
using namespace yrpc::detail;

typedef google::protobuf::Message Message;
typedef std::shared_ptr<Message> MessagePtr;

template<class ParamPackType,class ReturnPackType>
void register_service(std::string name,ServiceFunc func)
{
    uint32_t ret = yrpc::detail::ServiceMap::GetInstance()->insert(name,func,
    /*解析：字节流转化为message对象。 序列化：message对象转化为字节流*/
    [](bool is_parse,std::any& arg1,std::any& arg2){
        if(is_parse)
            arg2 = yrpc::detail::Codec::ParseToMessage<ParamPackType>(std::any_cast<std::string>(arg1));
        else //序列化
        {
            yrpc::detail::Codec::Serialize<ReturnPackType>(std::any_cast<google::protobuf::Message*>(arg1),std::any_cast<std::string&>(arg2));
        }
    });
    assert(ret >= 0);   //服务注册失败，大概率注册时导致的服务名冲突
}



// /* 
//     Codec 测试案例
// */
// using namespace google::protobuf;
// void test1()
// {
//     yrpc::detail::Codec codec;  //编码解码器

//     address addr;
//     addr.set_a(INT_MAX);
//     addr.set_b(true);
//     addr.set_c("hello world!");
//     addr.set_d(INT64_MAX);


//     std::string bytes;
//     codec.Serialize<address>(&addr,bytes);


//     Message* msg = new address;
//     //解析程序，返回包含所有数据的vector
//     auto ptr = codec.Parse<address>(bytes);
    
    
//     int res1 = std::any_cast<int>((*ptr)[0]);
//     printf("a: %d\n",res1);
//     bool res2 = std::any_cast<bool>((*ptr)[1]);
//     printf("b: %d\n",res2);
//     std::string res3 = std::any_cast<std::string>((*ptr)[2]);
//     printf("c: %s\n",res3.c_str());
//     int64 res4 = std::any_cast<int64>((*ptr)[3]);
//     printf("d: %ld\n",res4);

// }

void test2()
{
    
    register_service<addreq,addrsp>("add",[](std::any args){
        auto ptr = std::any_cast<std::shared_ptr<addreq>>(args);
        auto req = (addreq*)(ptr.get());

        int a = req->rint();
        int b = req->lint();
        auto rsp = new addrsp();
        rsp->set_result(a+b);
        printf("调用成功: %d + %d = %d\n",a,b,a+b);
        return rsp;   //返回值直接send
    });
    printf("服务注册完毕\n");
}

void calltest1(int a,int b)
{
    addreq req;
    req.set_rint(a);
    req.set_lint(b);
    std::string beforebytes;
    Codec::Serialize<addreq>(&req,beforebytes);

    
    std::any args = std::make_shared<addreq>();
    std::any bytes_ = beforebytes; 

    ServiceMap::GetInstance()->NameToService("add")->second(true,bytes_,args);

    auto msg = std::any_cast<std::shared_ptr<addreq>>(args);

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