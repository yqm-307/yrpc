#include "ServiceModule.h"
#include "ProtocolFactory.h"



using namespace yrpc::rpc::detail;



Service_Base::Service_Base()
{
}

Service_Base::~Service_Base()
{
}


void Service_Base::Dispatch(Buffer&& pck,RpcSessionPtr sess)
{
    Resolver resolve(pck);
    auto head = resolve.GetProtocolHead();
    MessagePtr send = nullptr;
    YRPC_PROTOCOL type = YRPC_PROTOCOL::type_S2C_RPC_ERROR;
    do
    {
        assert(pck.DataSize() >= ProtocolHeadSize);
        auto service = ServiceMap::GetInstance()->IdToService(resolve.GetServiceID());
        if (service == nullptr)
        {
            // 错误处理
            auto msg = DoErrHandler(RPC_ERRCODE::CALL_FATAL_SERVICE_ID_IS_BAD,"can't find service id");
            send = msg; type = YRPC_PROTOCOL::type_S2C_RPC_ERROR;
            break;
        }

        std::string msg_bytearray(pck.Peek() + ProtocolHeadSize, pck.DataSize() - ProtocolHeadSize);
        MessagePtr req_ptr;
        service->second(yrpc::detail::CodeOrService::Decode,req_ptr,msg_bytearray);

        //服务调用
        MessagePtr ret = service->first(
            req_ptr,
            sess
        );  //调用服务，问题：如果是非常耗时的操作，就会形成类似阻塞的效果，影响线程内主协程的调度
        if ( ret!= nullptr)
        {
            send = ret; type = YRPC_PROTOCOL::type_S2C_RPC_CALL_RSP;
            break;
        }
        else
        {
            auto msg = DoErrHandler(RPC_ERRCODE::CALL_FATAL_SERVICE_MSG_IS_BAD,"message retval is nullptr");
            send = msg; type = YRPC_PROTOCOL::type_S2C_RPC_ERROR;
            break;
        }
    } while (0);

    SendPacket(sess, send, head, type);
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


Service_Base* Service_Base::GetInstance()
{

    static Service_Base* ptr = nullptr;

    if(ptr == nullptr)
    {
        ptr = new Service_Base();
    }
    return ptr;
}


void Service_Base::SendPacket(RpcSessionPtr sess,MessagePtr pck,const ProtocolHead& head,YRPC_PROTOCOL type)
{
    Generater tobytes(pck,head,type);

    bbt::buffer::Buffer tmp;
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
        ERROR("[YRPC][Service_Base::SendPacket] peer {%s} is closed, send failed!",sess->GetPeerAddress().GetIPPort().c_str());
    }
}
