#include "Service.h"
#include "../protocol/all.h"
#include "../proto/yrpc_core_protocol/s2c.pb.h"
#include "../proto/yrpc_core_protocol/c2s.pb.h"
using namespace yrpc::rpc::detail;
using namespace yrpc::err;





std::string CallCenter::Service(const std::string_view& bytes)
{
    yrpc::detail::protocol::YProtocolResolver rsp(bytes);   // 解析bytes
    uint32_t uid = rsp.GetProtoID();
    uint16_t id = rsp.GetProtoType();


    // uint64_t uid =  yrpc::util::protoutil::BytesToType<uint64_t>(bytes.begin()+sizeof(int)*2);
    // uint32_t id = yrpc::util::protoutil::BytesToType<uint32_t>(bytes.begin()+sizeof(int));    //解析出服务id
    
    auto handles = yrpc::detail::ServiceMap::GetInstance()->IdToService(id);    //获取服务
    if(handles == nullptr)  //服务获取失败
    {    
        ERROR("CallCenter::Service() error ! Service id is bad!");
        std::string bytes{""};
        ErrCodeToByteArray(id,uid,yrpc::detail::protocol::define::CALL_FATAL_SERVICE_ID_IS_BAD,bytes,yrpc::util::logger::format("Service id is %d !",id));
        return bytes;
    }
    
    //解析出入参
    std::any byte_view_ = std::string_view(bytes.begin()+ProtocolHeadSize,bytes.size()-ProtocolHeadSize);
    std::any args;
    handles->second(true,byte_view_,args);  //Decode  将字节流转化为message对象


    //服务调用
    //返回msgtype 的message对象
    google::protobuf::Message* ret = handles->first(args);  //调用服务，问题：如果是非常耗时的操作，就会形成类似阻塞的效果，影响线程内主协程的调度


    //序列化

    std::string sendbyte('0',10); 
    std::any rlt = std::make_any<std::string>("");

    std::any res = ret;
    handles->second(false,res,rlt); //将res解析为字节流到rlt中
    
    sendbyte = std::any_cast<std::string>(rlt);
    // yrpc::detail::GenerateMsg(id,uid,sendbyte); 
    delete ret; //释放资源

    return std::any_cast<std::string>(sendbyte);
}




void AppendBKDRToArray(std::string& str,const char*data,uint32_t len)
{
    uint32_t hcode = yrpc::util::hash::BKDRHash(data,len);
    str.append((char*)(&hcode),sizeof(uint32_t));
}




int CallCenter::ErrCodeToByteArray(uint32_t id,uint32_t uid,int err,std::string& bytearray,std::string info)
{
    auto pck = std::make_shared<S2C_RPC_ERROR>();
    pck->set_errnocode(err);
    pck->set_info(info);
    yrpc::detail::protocol::YProtocolGenerater req(pck,yrpc::detail::protocol::define::type_S2C_RPC_ERROR);
    req.SetProtoID(uid);
    req.ToByteArray(bytearray);
    
    return 0;
}


