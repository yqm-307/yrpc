#pragma once
#include "Define.h"
// #include "Codec.h"

namespace yrpc::detail::protocol
{
// template<class ProtobufMsg> class Base_Msg;
/**
 * @brief 自定义协议的解析工具 响应 response
 *  1、自定义协议类型
 *  2、保存协议对象
 *  3、提供反序列化接口
 */
class YProtocolResolver: public Base_Msg<google::protobuf::Message>
{
    typedef yrpc::detail::protocol::define::YRPC_PROTOCOL YRPC_PROTOCOL;
    typedef google::protobuf::Message           Message;
    typedef std::shared_ptr<Message>            MessagePtr;

public:
    
    YProtocolResolver(std::string_view bytes):m_bytes(bytes) {}

    YProtocolResolver(const YProtocolResolver& p);

    virtual ~YProtocolResolver();


    /**
     * @brief 解析出协议对象
     * 
     * @param ProtobufRsp& message  协议对象传引用返回
     * @return true 
     * @return false 
     */
    bool ToProtoMsg(MessagePtr message) const 
    {
        m_protocol_head.SetByteArray(m_bytes.data());
        std::string_view proto_bytes(m_bytes.data(),m_bytes.size()-sizeof(uint16_t)*3);        
        return yrpc::detail::Codec::ParseToMessage(message,proto_bytes);
    }



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
    ProtocolHead        m_protocol_head;   //协议头
    YRPC_PROTOCOL       m_prototype;
    std::string         m_bytes;
};   

}