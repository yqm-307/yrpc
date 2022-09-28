#pragma once
#include "Codec.h"
#include "Protocol.h"
#include "../Util/logger.h"


namespace yrpc::detail
{


/**
 * @brief 自定义协议解析工具
 * 
 */
class YRPCProtocol
{
public:
    /**
     * @brief 对 YRPC_PROTOCOL_MESSAGE 数据部分进行解析
     * 
     * @tparam ProtoType protobuf Req对象类型
     * @param msg 消息数据字节流
     * @param len 长度
     * @param errcode 错误码
     * @return std::shared_ptr<ProtoType> 解析结果以智能指针传出 
     */
    template<class ProtoType>
    static std::shared_ptr<ProtoType> Message_Parse(std::string_view msg,size_t len,int* errcode);

    /**
     * @brief 对 YRPC_PROTOCOL_ERROR 数据部分进行解析
     * 
     * @tparam ErrType 错误数据返回类型
     * @param msg 
     * @param len 
     * @param errcode 
     * @return std::shared_ptr<ErrType> 
     */
    static void ErrMsg_Parse(std::string_view msg, size_t len, int *errcode)
    {
        using namespace yrpc::util::protoutil;

        assert(len >= sizeof(int));
        //错误字段，错误信息就是string?
        int error = BytesToType<int32_t>(msg.data());
        switch (error)
        {
        case CALL_FATAL_SERVICE_ID_IS_BAD:
            *errcode = -3;
            break;
        case CALL_FATAL_SERVICE_MSG_IS_BAD:
            *errcode = -4;
        case CALL_FATAL_SERVICE_TIMEOUT:
            *errcode = -5;
        case CALL_FATAL_SERVER_BUSY:
            *errcode = -6;
        default:
            *errcode = -7; //报文解析失败
            break;
        }
    }

    /**
     * @brief 传入预留16字节的字符串，将自动填充数据长度+服务id+uid
     *
     * @param service_id 服务id
     * @param uid   包id
     * @param bytes 要填充的buffer
     */
    static void GenerateMsg(uint16_t service_id, uint32_t uid, std::string &bytes)
    {
        using namespace yrpc::util::protoutil;
        yrpc::detail::protocol::ProtocolHead head;
        head.m_id = uid;
        head.m_type = service_id;
        char *p = bytes.data();
        type = type<<16;    //推到高位
        type += (uint16_t)bytes.size();
        ToBytes<int>(type, p);
        ToBytes<uint32_t>(service_id, p + sizeof(int));
        ToBytes<uint64_t>(uid, p + sizeof(int) * 2);
    }


    /**
     * @brief 协议解析，将整个协议包传入，解析成功返回只能指针对象，并设置错误码，失败返回nullptr，并设置错误码
     * 
     * @tparam RecvType 
     * @param msg 
     * @param len 
     * @param errcode 
     * @return std::shared_ptr<RecvType> 
     */
    template<class RecvType>
    static std::shared_ptr<RecvType> Protocol_Parse(std::string_view msg,size_t len,int* errcode)
    {   
        using namespace yrpc::util::protoutil;
        assert(len >= sizeof(int)*4);            

        int unioner = BytesToType<int>(msg.data());
        int length = len;   //包长度
        int msgtype = (unioner-length)>>16; //包数据类型
        uint64_t uid = BytesToType<uint64_t>(msg.data() + sizeof(int)*2);
        

        std::shared_ptr<RecvType> ptr;
        
        switch (msgtype)
        {
        case YRPC_PROTOCOL_MESSAGE:
            ptr = YRPCProtocol::Message_Parse<RecvType>(msg,len,errcode);
            return ptr;
            /*protobuf*/
        case YRPC_PROTOCOL_ERROR:
            YRPCProtocol::ErrMsg_Parse(msg,len,errcode);
            /*errcode*/
            break;
        default:
            FATAL("Codec::Protocol_Parse() error ! message type undefined!");
            /* DEFAULT */
            break;
        }
        return nullptr;
    }

};


template<class ProtoType>
std::shared_ptr<ProtoType> YRPCProtocol::Message_Parse(std::string_view msg,size_t len,int* errcode)
{
    using namespace yrpc::util::protoutil;

    //报文解析
    auto ptr = Codec::ParseToMessage<ProtoType>(msg);        
        
    if(!ptr->IsInitialized())
    {//解析失败
        ERROR("Codec::Protocol_Parse() error , message parse fatal!");
        *errcode = -2;  //protobuf解析失败
        return nullptr;
    }
    else    
        *errcode = 0;

    return ptr;
    
}


}