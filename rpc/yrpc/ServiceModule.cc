#include "ServiceModule.h"
#include "../msg/ProtocolFactory.h"



using namespace yrpc::rpc::detail;



Service_Base::Service_Base(bool b,int n)
    :m_canToService(b)
{
    if ( n <= 0)
        m_pool = nullptr;
    else
        m_pool = std::make_unique<ThreadPool>(n,65535);
}

Service_Base::~Service_Base()
{
}


void Service_Base::Dispatch(Buffer&& pck,RpcSessionPtr sess)
{

    assert(pck.DataSize() >= ProtocolHeadSize);
    Resolver resolve(pck);
    auto service = ServiceMap::GetInstance()->IdToService(resolve.GetServiceID());
    if (service == nullptr)
    {
        // 错误处理
        auto msg = DoErrHandler(RPC_ERRCODE::CALL_FATAL_SERVICE_ID_IS_BAD,"can't find service id");
        SendPacket(sess,msg,resolve.GetProtocolHead(),YRPC_PROTOCOL::type_S2C_RPC_ERROR);
        return;
    }


    std::string msg_bytearray(pck.Peek()+ProtocolHeadSize,pck.DataSize()-ProtocolHeadSize);
    // auto byte_view = std::string_view(pck.Peek()+ProtocolHeadSize,pck.DataSize()-ProtocolHeadSize);
    auto aa = yrpc::rpc::ProtocolFactroy::GetInstance()->Create(resolve.GetProtoID());
    MessagePtr req_ptr;
    service->second(yrpc::detail::CodeOrService::Decode,req_ptr,msg_bytearray);

    //服务调用
    //返回msgtype 的message对象
    service->first(req_ptr,
                        [this,sess,resolve,service](std::shared_ptr<google::protobuf::Message> Packet)->
    void{
        this->SendPacket(sess,Packet,resolve.GetProtocolHead(),YRPC_PROTOCOL::type_S2C_RPC_CALL_RSP);
    });  //调用服务，问题：如果是非常耗时的操作，就会形成类似阻塞的效果，影响线程内主协程的调度


}




Service_Base::MessagePtr Service_Base::DoErrHandler(RPC_ERRCODE type,const std::string& info)
{

    auto err = std::make_shared<S2C_RPC_ERROR>();
    err->set_errnocode(type);
    err->set_info(info);
    // std::string bytes;
    // err->SerializeToString(&bytes);
    return err;
}


Service_Base* Service_Base::GetInstance(bool open,int ths)
{

    static Service_Base* ptr = nullptr;

    if(ptr == nullptr)
    {
        if (ths <= 0)
            ptr = new Service_Base(open,0);
        else
            ptr = new Service_Base(open,ths);
    }
    return ptr;
}


void Service_Base::SendPacket(RpcSessionPtr sess,MessagePtr pck,const ProtocolHead& head,YRPC_PROTOCOL type)
{
    Generater tobytes(pck,head,type);

    yrpc::util::buffer::Buffer tmp;
    if ( !tobytes.ToByteArray(tmp) )
    {
        ERROR("code generate failed!");
    }

    if( !sess->IsClosed() )
    {
        sess->Append(tmp);
    }
    else
    {
        ERROR("peer {%s} is closed, send failed!",sess->GetPeerAddress().GetIPPort().c_str());
    }

}
