#pragma once
#include <iostream>
#include <functional>
#include <bbt/myhash/BKDR.hpp>
#include "CallObj.h"


namespace yrpc::rpc
{


class CallObjFactory
{
    typedef google::protobuf::Message   Message;
    typedef std::shared_ptr<Message>    MessagePtr;
    typedef yrpc::detail::protocol::YProtocolGenerater  Generater;  // 存储 request 并提供序列化
    typedef yrpc::detail::protocol::YProtocolResolver   Resolver;   // 存储 response bytearray 提供反序列化
    typedef std::function<void(MessagePtr,const std::string&)>    DeCodeFunc; // 解码
    typedef std::unordered_map<int,DeCodeFunc>      DeCodeMap;
    typedef bbt::buffer::Buffer                     Buffer;
public:
    typedef yrpc::detail::protocol::define::YRPC_PROTOCOL   YRPC_PROTOCOL;

    static CallObjFactory* GetInstance()
    {
        static CallObjFactory* instance = nullptr;
        if(instance == nullptr)
            instance = new CallObjFactory();
        return instance;
    }

    template<typename ReqType,typename RspType> 
    detail::CallObj::Ptr Create(ReqType&& msgptr,std::string&& servicename, detail::CallObj::CallResultFunc func);
    template<typename ReqType,typename RspType> 
    detail::CallObj::Ptr Create(ReqType&& msgptr,std::string&& name,YRPC_PROTOCOL type,detail::CallObj::CallResultFunc func);

    template<typename ReqType,typename RspType>
    std::pair<int, int> GetMessageTypeId();

    template<typename ReqType,typename RspType>
    void Register();
private:
    int GetIds();
private:
    std::atomic_int m_global_id{0};
};

template<typename ReqType,typename RspType>
detail::CallObj::Ptr CallObjFactory::Create(ReqType&& msg,std::string&& name,YRPC_PROTOCOL type,detail::CallObj::CallResultFunc func)
{
    /**
     * 这里就是利用模板的实例化，分别创建不同的函数实例。
     *   通过这种方式可以实现一个隐形的注册，我之前一直烦恼一个问题，就是如果让框架本身不需要去
     * 关注类型，或者说是想要一个反射一样的东西。但是最后发现不是很理想。
     *   否则就需要提供诸如 init( pair(protoid,messagetype) , ...)这样的接口，让使用者显示注册
     * 类型信息.
     * 但是利用函数模板的实例化，就可以让每种类型的函数都有一个独立的id，这样就不用再外侧去显式的注册
     * 类型信息。相当于函数第一次调用绑定了初始化，后续调用就不需要初始化了
     */
    auto [reqid, rspid] = GetMessageTypeId<ReqType, RspType>();
    auto mmptr = std::make_shared<ReqType>(std::move(msg));//ProtocolFactroy::GetInstance()->Create(local_id);
    Generater req(mmptr, bbt::hash::BKDR::BKDRHash(name), type);
    std::string tmp;
    
    Buffer buf;
    if (!req.ToByteArray(buf))
    {
        ERROR("Serialization failure!");
        return nullptr;
    }
    
    return detail::CallObj::Create(reqid, rspid, std::move(buf), type, func);
}

template<typename ReqType,typename RspType>
detail::CallObj::Ptr CallObjFactory::Create(ReqType&& msg,std::string&& name,detail::CallObj::CallResultFunc func)
{
    return Create<ReqType,RspType>(std::move(msg),std::move(name),YRPC_PROTOCOL::type_C2S_RPC_CALL_REQ,func);
}

template<typename ReqType,typename RspType>
std::pair<int, int> CallObjFactory::GetMessageTypeId()
{
    static int message_id = 0; 
    if( message_id == 0 )
    {
        auto id_req = GetIds();
        message_id = id_req;
        ProtocolFactroy::GetInstance()->Insert({
            YRPC_PROTO_REGISTER( message_id         , ReqType ),
            YRPC_PROTO_REGISTER( (message_id + 1)   , RspType ),
        });
    }
    return std::pair<int,int>(message_id, message_id + 1);
}

template<typename ReqType,typename RspType>
void CallObjFactory::Register()
{
    GetMessageTypeId<ReqType, RspType>();
}

}