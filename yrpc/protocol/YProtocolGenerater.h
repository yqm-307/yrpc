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
    typedef bbt::buffer::Buffer                 Buffer;
public:
    typedef Base_Msg<google::protobuf::Message> BaseProtocol;
    typedef typename std::shared_ptr<YProtocolGenerater> Ptr;
    

    // 库自用的service id 为0  
    YProtocolGenerater(MessagePtr req,uint32_t serviceid,YRPC_PROTOCOL prototype);
    YProtocolGenerater(MessagePtr req,const ProtocolHead& head,YRPC_PROTOCOL prototype);

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
    bool ToByteArray(Buffer& bytearray) const;
    
    template<typename PckType>
    static bool ToByteArray(Message* msg,Buffer& bytearray)
    {
        std::string tmp{""};
        tmp.resize(ProtocolHeadSize, '0');

        if(msg == nullptr)
            return false;
        if (!yrpc::detail::Codec::Serialize<PckType>(msg, tmp))
            return false;
        bytearray = std::move(Buffer(tmp));
        
        return true;
    }


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
    { m_protocol_head.m_type = type; }



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



protected:
    mutable ProtocolHead m_protocol_head;           //协议头
    MessagePtr m_message;
};    



}