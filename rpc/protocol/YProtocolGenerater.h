#pragma once
#include "Define.h"
// #include "../Util/IDGenerate.h"
#include <memory>

namespace yrpc::detail::protocol
{



/**
 * @brief 非常好用的针对yrpc protocol 的协议序列化工具
 * 要点: 
 *  1、只需要填充 prototype，协议头自动填充。如果不自定义协议头，则会自动生成uid和
 *  2、自动填充协议头
 *  3、一键序列化
 */
class YProtocolGenerater: public Base_Msg<google::protobuf::Message>
{
public:
    typedef Base_Msg<google::protobuf::Message> BaseProtocol;
    typedef typename std::shared_ptr<YProtocolGenerater> Ptr;
    
    virtual ~YProtocolGenerater(){}    
    YProtocolGenerater(const YProtocolGenerater& p)
            :m_prototype(p.m_prototype),
            m_protocol_head(p.m_protocol_head),
            m_message(p.m_message)
    {}


    YProtocolGenerater()
            :m_message(nullptr)
    {}

    YProtocolGenerater(google::protobuf::Message* req,YRPC_PROTOCOL prototype)
            :m_prototype(prototype),
            m_message(req)
    {}
    YProtocolGenerater& operator=(const YProtocolGenerater& p)
    {
        m_protocol_head = p.m_protocol_head;
        m_message = m_message;
        m_prototype = m_prototype;
    }


    // YProtocolGenerater(MessagePtr& rsp):m_prototype(PType),BaseProtocol(rsp){}
    


public:


    /**
     * @brief YProtocolGenerater 序列化为字节流，并以string形式返回
     * 
     * @param std::string& bytearray 
     * @return true 
     * @return false 
     */
    bool ToByteArray(std::string& bytearray)
    {
        bytearray.resize(ProtocolHeadSize,'0'); // 预留 protocol head
        
        if(this->Encode(m_message,bytearray))   // 追加   
        {
            // int msglen = bytearray.size();
            m_protocol_head.m_length = bytearray.size();    // 协议长
            m_protocol_head.ToByteArray(bytearray.data());  
            // m_protocol_head.m_type = ;
            return true;
        }
        else    
            return false;
    }

    void SetMessage(google::protobuf::Message* rsp)
    { m_message = rsp; }
    void SetProtoType(YRPC_PROTOCOL type)
    { m_prototype = type; }
    /**
     * @brief 返回原始 protobuf 协议对象
     * 
     * @return MessagePtr protobuf协议对象的智能指针 
     */
    google::protobuf::Message* Message()
    { return m_message; }

    /**
     * @brief 返回协议proto id
     * 
     * @return uint32_t 协议id
     */
    uint32_t GetProtoID()
    { return m_protocol_head.m_id; }

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
    static Ptr Create(YRPC_PROTOCOL type,google::protobuf::Message* proto = nullptr)
    {
        return std::make_shared<YProtocolGenerater>(proto,type);
    }

protected:
    // 32位长
    ProtocolHead m_protocol_head;           //协议头
    YRPC_PROTOCOL m_prototype;
    google::protobuf::Message* m_message;
};    



}