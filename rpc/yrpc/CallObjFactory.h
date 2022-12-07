#include <iostream>
#include "CallObj.h"
#include <functional>


namespace yrpc::rpc
{


class CallObjFactory
{
    typedef google::protobuf::Message   Message;
    typedef std::shared_ptr<Message>    MessagePtr;

    typedef std::function<void(MessagePtr,const std::string&)>    DeCodeFunc; // 解码
    typedef std::unordered_map<int,DeCodeFunc>      DeCodeMap;
    typedef yrpc::detail::protocol::define::YRPC_PROTOCOL   YRPC_PROTOCOL;

public:

    static CallObjFactory* GetInstance()
    {
        static CallObjFactory* _instance = nullptr;
        if(_instance == nullptr)
            _instance = new CallObjFactory();
        return _instance;
    }

    template<typename ReqType,typename RspType> 
    detail::CallObj::Ptr Create(const ReqType& msgptr,std::string&& servicename, detail::CallObj::CallResultFunc func);
    template<typename ReqType,typename RspType> 
    detail::CallObj::Ptr Create(const ReqType& msgptr,std::string&& name,YRPC_PROTOCOL type,detail::CallObj::CallResultFunc func);
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
detail::CallObj::Ptr CallObjFactory::Create(const ReqType& msg,std::string&& name,YRPC_PROTOCOL type,detail::CallObj::CallResultFunc func)
{
    static int local_id = 0;  // 用来判断是否第一次进入函数,第一次进入执行一下注册操作   
    if (local_id == 0)
    {
        local_id = global_id;
        global_id.fetch_add(2);
        // 注册类型到工厂
        ProtocolFactroy::GetInstance()->Insert({
            YRPC_PROTO_REGISTER( local_id , ReqType ),
            YRPC_PROTO_REGISTER( (local_id+1) , RspType ),
        });
    }
    auto mmptr = std::make_shared<ReqType>(msg);//ProtocolFactroy::GetInstance()->Create(local_id);
    uint32_t hashid = yrpc::util::hash::BKDRHash(name);
    
    return detail::CallObj::Create<ReqType>(mmptr,local_id,hashid,type,func);
}

template<typename ReqType,typename RspType>
detail::CallObj::Ptr CallObjFactory::Create(const ReqType& msgptr,std::string&& name,detail::CallObj::CallResultFunc func)
{
    return Create<ReqType,RspType>(msgptr,std::move(name),YRPC_PROTOCOL::type_C2S_RPC_CALL_REQ,func);
}

}