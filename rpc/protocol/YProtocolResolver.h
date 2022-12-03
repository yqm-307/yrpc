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
    typedef yrpc::util::buffer::Buffer          Buffer;
public:
    
    YProtocolResolver(const Buffer& bytes);
    YProtocolResolver(Buffer&& bytes);

    YProtocolResolver(){};
    YProtocolResolver(const YProtocolResolver& p);
    virtual ~YProtocolResolver();

    YProtocolResolver(YProtocolResolver&& rval);
    YProtocolResolver &operator=(YProtocolResolver&& rval);
    YProtocolResolver &operator=(const YProtocolResolver& rval);

    /**
     * @brief 解析出协议对象
     * 
     * @param ProtobufRsp& message  协议对象传引用返回
     * @return true 
     * @return false 
     */
    bool ToProtoMsg(MessagePtr message) const;



    /**
     * @brief 返回协议proto id(需要先DeCode)
     * 
     * @return uint32_t 协议id
     */
    uint32_t GetProtoID() const 
    { return m_protocol_head.m_id; }


    /**
     * @brief 获取服务id(需要先DeCode)
     * 
     * @return uint32_t 
     */
    uint32_t GetServiceID() const
    { return m_protocol_head.m_serviceid; }



    /**
     * @brief 获取协议类型(需要先DeCode)
     * 
     * @return YRPC_PROTOCOL 协议类型枚举 
     */
    YRPC_PROTOCOL GetProtoType() const 
    { return (YRPC_PROTOCOL)m_protocol_head.m_type; }


    /**
     * @brief 设置待解析数据字节流
     * 
     * @param view 
     */
    void SetByteArray(const Buffer& view) 
    { m_bytes = view; }
    void SetByteArray(Buffer&& view) 
    { m_bytes =std::move(view); }

    /**
     * @brief 待解析字节流是否为空
     * 
     * @return true 
     * @return false 
     */
    bool IsEmpty()
    { return m_bytes.DataSize() == 0; }


    const ProtocolHead& GetProtocolHead() const
    { return m_protocol_head; }
protected:
    void ParseHead();

protected:
    // 32位长
    ProtocolHead        m_protocol_head;   //协议头
    // YRPC_PROTOCOL       m_prototype;
    Buffer              m_bytes;
};   

}