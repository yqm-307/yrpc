#pragma once
#include "Define.h"
#include <memory>

namespace yrpc::detail::protocol
{



/**
 * @brief 自定义协议 请求 request
 *  
 *  1、存储协议对象
 *  2、提供必要接口(序列化接口)
 */
class YProtocolGenerater: public Base_Msg<google::protobuf::Message>
{
    typedef yrpc::detail::protocol::define::YRPC_PROTOCOL YRPC_PROTOCOL;
    typedef google::protobuf::Message           Message;
    typedef std::shared_ptr<Message>            MessagePtr;

public:
    typedef Base_Msg<google::protobuf::Message> BaseProtocol;
    typedef typename std::shared_ptr<YProtocolGenerater> Ptr;
    

        
    YProtocolGenerater(MessagePtr req,YRPC_PROTOCOL prototype);

    virtual ~YProtocolGenerater(){}    


    YProtocolGenerater(const YProtocolGenerater& p);
    YProtocolGenerater& operator=(const YProtocolGenerater& p);    
    YProtocolGenerater() = delete;
public:


    /**
     * @brief YProtocolGenerater 序列化为字节流，并以string形式返回
     * 
     * @param std::string& bytearray 
     * @return true 
     * @return false 
     */
    bool ToByteArray(std::string& bytearray) const;


    /**
     * @brief 设置 protobuf 
     * 
     * @param rsp 
     */
    void SetMessage(MessagePtr rsp) 
    { m_message = rsp; }
    
    

    /**
     * @brief 设置protobuf类型, 类型枚举为 yrpc::detail::protocol::define::YRPC_PROTOCOL
     * 
     * @param type YRPC_PROTOCOL 枚举值
     */
    void SetProtoType(YRPC_PROTOCOL type)
    { m_prototype = type; }



    /**
     * @brief 返回原始 protobuf 智能指针
     * 
     * @return MessagePtr protobuf协议对象的智能指针 
     */
    // MessagePtr Message() = delete;



    /**
     * @brief 返回包 id (10分钟内唯一)
     * 
     * @return uint32_t 包id
     */
    uint32_t GetProtoID()
    { return m_protocol_head.m_id; }



    /**
     * @brief 自己定义包id ,在ToByteArray之前设置,否则无效
     * 
     * @param id 
     */
    void SetProtoID(uint32_t id)
    { m_protocol_head.m_id = id; }



    /**
     * @brief 获取协议类型
     * 
     * @return YRPC_PROTOCOL 协议类型枚举 
     */
    YRPC_PROTOCOL GetProtoType()
    { return (YRPC_PROTOCOL)m_protocol_head.m_type; }



    /**
     * @brief 创建一个 Request 对象
     * 
     * @param ProtobufRsp proto 
     * @return Ptr 智能指针 
     */
    static Ptr Create(YRPC_PROTOCOL type,MessagePtr proto = nullptr)
    { return std::make_shared<YProtocolGenerater>(proto,type); }

protected:
    mutable ProtocolHead m_protocol_head;           //协议头
    YRPC_PROTOCOL m_prototype;
    MessagePtr m_message;
};    



}