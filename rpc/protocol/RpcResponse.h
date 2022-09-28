#pragma once
#include "Define.h"


namespace yrpc::detail::protocol
{





class RpcResponse: public Base_Msg<google::protobuf::Message>
{
public:
    typedef google::protobuf::Message ProtobufRsp;
    RpcResponse(YRPC_PROTOCOL type,std::string_view bytes):m_bytes(bytes),m_prototype(type) {}
    RpcResponse(const RpcResponse& p)
            :m_prototype(p.m_prototype),
            m_protocol_head(p.m_protocol_head),
            m_message(p.m_message)
    {}
    virtual ~RpcResponse(){}

    


    /**
     * @brief 解析出协议对象
     * 
     * @param ProtobufRsp& message  协议对象传引用返回
     * @return true 
     * @return false 
     */
    bool ToProtoMsg(std::shared_ptr<ProtobufRsp> message) const 
    {
        //拆解head
        m_protocol_head.SetByteArray(m_bytes.data());
        std::string_view proto_bytes(m_bytes.data(),m_bytes.size()-sizeof(uint16_t)*3);
        return yrpc::detail::Codec::ParseToMessage(message,proto_bytes);
    }


    /**
     * @brief 返回原始 protobuf 协议对象
     * 
     * @return MessagePtr protobuf协议对象的智能指针 
     */
    ProtobufRsp* Message() const 
    { return m_message; }

    /**
     * @brief 返回协议proto id
     * 
     * @return uint32_t 协议id
     */
    uint32_t GetProtoID() const 
    { return m_protocol_head.m_id; }

    /**
     * @brief 获取协议类型
     * 
     * @return YRPC_PROTOCOL 协议类型枚举 
     */
    YRPC_PROTOCOL GetProtoType() const 
    { return (YRPC_PROTOCOL)m_protocol_head.m_type; }

protected:
    // 32位长
    ProtocolHead m_protocol_head;   //协议头
    YRPC_PROTOCOL m_prototype;
    std::string m_bytes;
    google::protobuf::Message* m_message;

};   

}