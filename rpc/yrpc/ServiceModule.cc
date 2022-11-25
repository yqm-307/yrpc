#include "ServiceModule.h"


using namespace yrpc::rpc::detail;



Service_Base::Service_Base(bool b,int n)
    :m_pool(std::make_unique<ThreadPool>(n,65535)),
    m_canToService(b)
{
}

Service_Base::~Service_Base()
{
}


void Service_Base::Dispatch(const std::string& pck,RpcSessionPtr sess)
{

    assert(pck.size() >= ProtocolHeadSize);
    Resolver rsl(pck);
    std::string sret;

    auto service = ServiceMap::GetInstance()->IdToService(rsl.GetServiceID());
    if (service == nullptr)
    {
        // 错误处理
        sret = DoErrHandler(RPC_ERRCODE::CALL_FATAL_SERVICE_ID_IS_BAD,"can't find service id");
    }


    std::any byte_view = std::string_view(pck.c_str()+ProtocolHeadSize,pck.size()-ProtocolHeadSize);
    std::any args;
    service->second(yrpc::detail::CodeOrService::Service,byte_view,args);

    //服务调用
    //返回msgtype 的message对象
    google::protobuf::Message* ret = service->first(args);  //调用服务，问题：如果是非常耗时的操作，就会形成类似阻塞的效果，影响线程内主协程的调度


    //序列化
    
    std::any rlt = std::make_any<std::string>("");
    std::any res = ret;
    /**
     * 关于为什么这里需要用异常捕获？
     * 
     * 这里如果出现问题是可以容忍的，比如协议格式商量错误。所以处理掉就可以了.
     * 前面 service handler 不归框架管，错误就是用户代码的问题
     */
    try
    {
        service->second(yrpc::detail::CodeOrService::Codec,res,rlt); //将res解析为字节流到rlt中
        sret = std::any_cast<std::string>(rlt);
    }
    catch(const std::exception& e)
    {
        FATAL("ServiceModule fatal ,%d",e.what());
        sret = DoErrHandler(RPC_ERRCODE::CALL_FATAL_SERVICE_MSG_IS_BAD , "rpcserver codec error!");
    }


    // yrpc::detail::GenerateMsg(id,uid,sendbyte); 
    delete ret; //释放资源

    // 发送结果
    sess->Append(sret);
}




std::string Service_Base::DoErrHandler(RPC_ERRCODE type,const std::string& info)
{

    auto err = std::make_shared<S2C_RPC_ERROR>();
    err->set_errnocode(type);
    err->set_info(info);
    std::string bytes;
    err->SerializeToString(&bytes);
    return bytes;
}


Service_Base* Service_Base::GetInstance(bool open,int ths)
{
    static Service_Base* ptr = nullptr;
    if(ptr == nullptr)
    {
        ptr = new Service_Base(open,ths);
    }
    return ptr;
}
