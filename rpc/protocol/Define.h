/**
 * @file Define.h
 * @author your name (you@domain.com)
 * @brief protocol 枚举定义
 * @version 0.1
 * @date 2022-09-10
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#pragma once
#include "../Util/all.h"
#include "Codec.h"
#include "../proto/c2s.pb.h"
#include "../proto/s2c.pb.h"
#include <assert.h>
#include <unordered_map>

namespace yrpc::detail::protocol::define
{

    // yrpc 错误码
    enum YRPC_ErrCode
    {
        CALL_FATAL_OTHRE = 0,                   // 未知错误
        CALL_FATAL_SERVICE_ID_IS_BAD    = 1,    //服务不存在或服务id错误
        CALL_FATAL_SERVICE_MSG_IS_BAD   = 2,    // protobuf message 错误
        CALL_FATAL_SERVICE_TIMEOUT      = 3,    // service call 超时
        CALL_FATAL_SERVER_BUSY          = 4     //服务端拒绝调用，服务端繁忙

    };

    /**
     * @brief yrpc 内部自定义protobuf类型集合
     *
     */
    enum YRPC_PROTOCOL
    {
        type_YRPC_PROTOCOL_Done = 0,
        type_C2S_HEARTBEAT_REQ = 10000,
        type_S2C_HEARTBEAT_RSP = 10001,
        type_C2S_RPC_CALL_REQ = 10010,
        type_S2C_RPC_CALL_RSP = 10011,
        type_S2C_RPC_ERROR = 10012,
    };

}

namespace yrpc::detail::protocol
{













//////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////



template<class ProtobufMsg>
class Base_Msg
{
public:
protected:

    /**
     * @brief 序列化后数据追加到到msg
     * 
     * @param std::string& msg 
     * @return true 
     * @return false 
     */
    virtual bool Encode(ProtobufMsg* proto,std::string& msg) const 
    {
        if(proto != nullptr)
            if (yrpc::detail::Codec::Serialize<ProtobufMsg>(proto,msg))
                return true;
        else
            return false;
    }
   
    /**
     * @brief 解序列化出msg
     * 
     * @param msg 
     * @return true 
     * @return false 
     */
    virtual bool Decode(std::shared_ptr<ProtobufMsg> proto,const std::string& msg) const
    {
        if(proto == nullptr)
            return false;    
        return yrpc::detail::Codec::ParseToMessage(proto,msg);
        
        // if (!proto)
        //     return false;
        // return true;
    }   

};









//////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * 
 * 
 *  |               |               |                       |                     |
 *  | length(16bit) |  type(16bit)  | protocol id(32bit)    | protobuf bytes(data)|    
 *  |               |               |                       |                     |
 *   包长度 2 字节  ,  范围 1-65535 
 *   协议类型 2 字节 , 范围 1-65535
 *   协议id 4 字节  ,  范围 1-42E
 */
#define ProtocolHeadSize (sizeof(uint16_t)+sizeof(uint16_t)+sizeof(uint32_t))
struct ProtocolHead
{
    ProtocolHead():m_type(0),m_length(0),m_id(yrpc::util::id::GenerateID::GetIDuint32()){}
    ProtocolHead(uint16_t type,uint16_t len,uint32_t id = yrpc::util::id::GenerateID::GetIDuint32())
        :m_type(type),m_length(len),m_id(id) {}
    ProtocolHead(const ProtocolHead& p):m_type(p.m_type),m_length(p.m_length),m_id(p.m_id) {}
    ~ProtocolHead(){}

    ProtocolHead& operator=(const ProtocolHead&p)
    { m_type = p.m_type;m_length=p.m_length;m_id=p.m_id; }


    /**
     * @brief 转换为byte流
     * 
     * @return std::string 
     */
    bool ToByteArray(char* start)
    {
        memcpy(start,(void*)&m_type,sizeof(uint16_t));  // 2 byte
        memcpy(start+sizeof(uint16_t),(void*)&m_length,sizeof(uint16_t)); // 2 byte
        memcpy(start+2*sizeof(uint16_t),(void*)&m_id,sizeof(uint32_t));     // 4 byte
        return true;
    }

    /**
     * @brief 自动将比特流解析出结果
     * 
     * @param const char* start 起始数据 
     */
    void SetByteArray(const char* start) const 
    {
        memcpy((void*)&m_type,start,sizeof(uint16_t));
        memcpy((void*)&m_length,start+sizeof(uint16_t),sizeof(uint16_t));
        memcpy((void*)&m_id,start+sizeof(uint16_t)*2,sizeof(uint32_t));
    }

    /**
     * @brief 转换为字符串
     * 
     * @return std::string 
     */
    std::string ToString() const
    {
        return std::to_string(m_type)+std::to_string(m_length)+std::to_string(m_id);
    }



    uint16_t m_type;      /* 协议类型 */
    uint16_t m_length;    /* 包长 */
    uint32_t m_id;        /* id */
};






}