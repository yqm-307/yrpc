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
public:

    template<typename ReqType,typename RspType>
    CallObj::Ptr Create(std::shared_ptr<ReqType> msgptr,CallObj::CallResultFunc func);

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
CallObj::Ptr CallObjFactory::Create(std::shared_ptr<ReqType> msgptr,CallObj::CallResultFunc func)
{
    static int local_id = 0;  // 静态变量有妙用,用来判断是否第一次进入函数,第一次进入执行一下注册操作
    if (local_id == 0)
    {
        local_id = global_id;
        global_id.fetch_add(2);
        // 注册类型到工厂
        ProtocolFactroy::GetInstance()->Insert(
            YRPC_PROTO_REGISTER( local_id , ReqType ),
            YRPC_PROTO_REGISTER( (local_id+1) , RspType ),
        );
    }
    return CallObj::Create<MsgType>(msgptr,local_id,func);
}


}