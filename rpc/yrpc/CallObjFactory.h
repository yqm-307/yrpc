#include <iostream>
#include "CallObj.h"
#include <functional>


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
    typedef yrpc::detail::protocol::define::YRPC_PROTOCOL   YRPC_PROTOCOL;
    typedef yrpc::util::buffer::Buffer                  Buffer;

public:

    static CallObjFactory* GetInstance()
    {
        static CallObjFactory* _instance = nullptr;
        if(_instance == nullptr)
            _instance = new CallObjFactory();
        return _instance;
    }

    template<typename ReqType,typename RspType> 
    detail::CallObj::Ptr Create(ReqType&& msgptr,std::string&& servicename, detail::CallObj::CallResultFunc func);
    template<typename ReqType,typename RspType> 
    detail::CallObj::Ptr Create(ReqType&& msgptr,std::string&& name,YRPC_PROTOCOL type,detail::CallObj::CallResultFunc func);
    // void test()
    // {
    //     m_idtocodec_map.insert(std::make_pair(1,[](MessagePtr ptr,const std::string& bytes){

    //         ptr->ParseFromString(bytes);
    //         ProtocolFactroy::GetInstance()->Insert(YRPC_PROTO_REGISTER(1,T));
    //     }));
    // }

private:
    DeCodeMap       m_idtocodec_map;
    static std::atomic_int  global_id;
};

std::atomic_int  CallObjFactory::global_id = 1;




template<typename ReqType,typename RspType>
detail::CallObj::Ptr CallObjFactory::Create(ReqType&& msg,std::string&& name,YRPC_PROTOCOL type,detail::CallObj::CallResultFunc func)
{
    /**
     * 这里就是利用模板的实例化，分别创建不同的函数实例。
     *   通过这种方式可以实现一个隐形的注册，我之前一直烦恼一个问题，就是如果让框架本身不需要去
     * 关注类型，或者说是想要一个反射一样的东西。但是最后发现不是很理想。就是有时候需要直到类型
     * （protobuf的类型信息），所以就用这个办法，实现一个类似的功能。
     *   否则就需要提供诸如 init( pair(protoid,messagetype) , ...)这样的接口，让使用者显示注册
     * 类型信息
     */
    static int local_id = 0;  
    if (local_id == 0)
    {
        local_id = global_id; 
        global_id.fetch_add(2);
        ProtocolFactroy::GetInstance()->Insert({
            YRPC_PROTO_REGISTER( local_id , ReqType ),
            YRPC_PROTO_REGISTER( (local_id+1) , RspType ),
        });
    }
    auto mmptr = std::make_shared<ReqType>(std::move(msg));//ProtocolFactroy::GetInstance()->Create(local_id);
    Generater req(mmptr,yrpc::util::hash::BKDRHash(name),type);
    std::string tmp;
    
    Buffer buf;
    if (!req.ToByteArray(buf))
    {
        ERROR("Serialization failure!");
        return nullptr;
    }
    
    return detail::CallObj::Create(local_id,local_id+1,std::move(buf),type,func);
}

template<typename ReqType,typename RspType>
detail::CallObj::Ptr CallObjFactory::Create(ReqType&& msg,std::string&& name,detail::CallObj::CallResultFunc func)
{
    return Create<ReqType,RspType>(std::move(msg),std::move(name),YRPC_PROTOCOL::type_C2S_RPC_CALL_REQ,func);
}

}