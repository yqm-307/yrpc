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
#include "../proto/yrpc_core_protocol/c2s.pb.h"
#include "../proto/yrpc_core_protocol/s2c.pb.h"
#include <assert.h>
#include <unordered_map>

namespace yrpc::detail::protocol::define
{

    // yrpc 错误码
    enum YRPC_ErrCode : int32_t
    {
        CALL_FATAL_OTHRE = 0,                   //  未知错误
        CALL_FATAL_SERVICE_ID_IS_BAD    = 1,    //  服务不存在或服务id错误
        CALL_FATAL_SERVICE_MSG_IS_BAD   = 2,    //  protobuf message 错误
        CALL_FATAL_SERVICE_TIMEOUT      = 3,    //  service call 超时
        CALL_FATAL_SERVER_BUSY          = 4     //  服务端拒绝调用，服务端繁忙

    };

    /**
     * @brief yrpc 使用的协议集合
     *
     * 在 YRPC 中，使用到的所有协议类型，都会在下面定义，而且协议类型包含在 protocol head 里面。
     * 就是说所有 YRPC 内部（用户自定义 protocol buffer 不算内部协议，因为进入YRPC已经序列化为
     * 字节流，或者说都算是 RPC_CALL_RSP 或者 RPC_CALL_REQ）使用的协议，都在下面的枚举值中。
     * 
     * 如果需要判断详细的req、rsp类型，就需要 google protobuf 相关api支持
     * 
     * 约定大于30000是 服务端响应
     */
    enum YRPC_PROTOCOL : uint16_t
    {
        type_YRPC_PROTOCOL_Done = 0,
    
        type_C2S_HEARTBEAT_REQ = 10000,
        type_C2S_RPC_CALL_REQ = 10010,

        /* 用来区分 c2s 和 s2c 协议的分界线*/
        /* 小于30000的都是请求,大于30000的都是响应 */ 
#define type_YRPC_PROTOCOL_CS_LIMIT 30000   
        type_S2C_HEARTBEAT_RSP = 30001,
        type_S2C_RPC_CALL_RSP = 30011,
        type_S2C_RPC_ERROR = 30012,         
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
    typedef std::shared_ptr<ProtobufMsg>    MessagePtr;
protected:

    /**
     * @brief 序列化后数据追加到到msg
     * 
     * @param std::string& msg 
     * @return true 
     * @return false 
     */
    virtual bool Encode(MessagePtr proto,std::string& msg) const 
    {        
        // bool result{false};
        // std::string str{""};
        // do{
        //     if( proto == nullptr )
        //         break;
        //     yrpc::detail::Codec::Serialize<ProtobufMsg>(proto,str);

        //     if ( yrpc::detail::Codec::Serialize<ProtobufMsg>(proto,msg) )
        //         result = true;
        // }while(0);

        // return result;
        bool result{false};
        do{
            if( proto == nullptr )
                break;
            if ( yrpc::detail::Codec::Serialize(proto,msg) )
                result = true;
        }while(0);

        return result;
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
 *  |               |               |                       |                   |                    |
 *  | length(16bit) |  type(16bit)  | BKDR ID(32bit)        | protocol id(32bit)|protobuf bytes(data)|    
 *  |               |               |                       |                   |                    |
 *   包长度 2 字节  ,  范围 1-65535     整条协议，包括协议头长度
 *   协议类型 2 字节,  范围 1-65535     定义在 YRPC_PROTOCOL 
 *   服务名 4 字节  ,  范围 1-42E       通过BKDR生成
 *   协议id 4 字节  ,  范围 1-42E       id generate 产生
 */
#define ProtocolHeadSize (sizeof(uint16_t)+sizeof(uint16_t)+sizeof(uint32_t)+sizeof(uint32_t))
#define ProtocolMaxSize UINT16_MAX
#define Protocol_LengthType         uint16_t
#define Protocol_YRPCCallType       uint16_t
#define Protocol_ServiceIdType      uint32_t
#define Protocol_PckIdType           uint32_t


struct ProtocolHead
{
    ProtocolHead() 
        :m_length(0), 
         m_type(define::type_YRPC_PROTOCOL_Done),
         m_serviceid(0),
         m_id(yrpc::util::id::GenerateID::GetIDuint32())
    {}
    ProtocolHead(define::YRPC_PROTOCOL type, uint16_t len,uint32_t sid,uint32_t id = yrpc::util::id::GenerateID::GetIDuint32())
        :m_length(len), 
         m_type(type),
         m_serviceid(sid),
         m_id(id) 
    {}
    ProtocolHead(const ProtocolHead &p) 
        :m_length(p.m_length), 
         m_type(p.m_type), 
         m_serviceid(p.m_serviceid), 
         m_id(p.m_id) 
    {}
    
    ProtocolHead(ProtocolHead&& p)
    { *this = std::move(p); }
    ~ProtocolHead() {}

    ProtocolHead &operator=(const ProtocolHead &p)
    {
        m_type = p.m_type;
        m_length = p.m_length;
        m_serviceid = p.m_serviceid;
        m_id = p.m_id;
        return *this;
    }

    ProtocolHead &operator=(ProtocolHead&& p)
    {
        m_type = p.m_type;
        m_length = p.m_length;
        m_serviceid = p.m_serviceid;
        m_id = p.m_id;
        p.Clear();
        return *this;
    }

    /**
     * @brief 将 ProtocolHead 编码为字节流到 start 中
     *
     * @return std::string start至少长 sizeof(ProtocolHead)
     */
    void EnCode(char *start) const
    {
        memcpy( start,
                (char*)&m_length,
                sizeof(Protocol_LengthType)); // 2 byte  [length]
        memcpy( start + sizeof(Protocol_LengthType),
                (char*)&m_type,
                sizeof(Protocol_YRPCCallType));                                                      // 2 byte  [type]
        memcpy( start + sizeof(Protocol_LengthType)+sizeof(Protocol_YRPCCallType),
                (char*)&m_serviceid, 
                sizeof(Protocol_ServiceIdType)); // 4 bytes [service id]
        memcpy( start + sizeof(Protocol_LengthType)+sizeof(Protocol_YRPCCallType)+sizeof(Protocol_ServiceIdType), 
                (char*)&m_id, 
                sizeof(Protocol_PckIdType));            // 4 byte  [package id]
    }

    /**
     * @brief 将start 解码到ProtocolHead
     *
     * @param const char* start 至少长 sizeof(ProtocolHead) 字节流
     */
    void DeCode(const char *start) const
    {
        memcpy( (char*)&m_length,
                start,
                sizeof(Protocol_LengthType));  // 2 bytes   [length]
        memcpy( (char*)&m_type,
                start + sizeof(Protocol_LengthType),
                sizeof(Protocol_YRPCCallType));                       // 2 bytes   [type]
        memcpy( (char*)&m_serviceid,
                start + sizeof(Protocol_LengthType) +sizeof(Protocol_YRPCCallType),
                sizeof(Protocol_ServiceIdType));
        memcpy( (char*)&m_id,
                start + sizeof(Protocol_LengthType) +sizeof(Protocol_YRPCCallType)+sizeof(Protocol_ServiceIdType),
                sizeof(Protocol_PckIdType));  // 4 bytes   [package id]
    }

    /**
     * @brief 转换为字符串
     *
     * @return std::string
     */
    std::string ToString() const
    {
        return std::to_string(m_type) + std::to_string(m_length) +std::to_string(m_serviceid)+ std::to_string(m_id);
    }


    void Clear()
    {
        m_length = 0;
        m_type = define::YRPC_PROTOCOL::type_YRPC_PROTOCOL_Done;
        m_serviceid = 0;
        m_id = 0;
    }

    uint16_t m_length;              /* 此字段表示这个包的总长度 */
    define::YRPC_PROTOCOL m_type;   /* 此字段表示这个包的协议类型 */
    uint32_t m_serviceid;           /* 此字段表示这个包所call的远程服务id */
    uint32_t m_id;                  /* 此字段表示这个包的id，标识这个包10分钟内唯一 */
};
}