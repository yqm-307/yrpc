#pragma once
#include "../network/TcpServer.h"
#include "../msg/servicemap.h"
#include "../protocol/ProtocolUtil.h"
#include "../Util/all.h"
#include "../protocol/ErrCode.h"
#include "../protocol/YProtocolResolver.h"
#include "../protocol/YProtocolGenerater.h"


namespace yrpc::rpc::detail
{
class CallCenter
{
    typedef yrpc::detail::protocol::YProtocolGenerater  Generater;
    typedef yrpc::detail::protocol::YProtocolResolver   Resolver;
    typedef google::protobuf::Message                   Message;
    typedef std::shared_ptr<Message>                    MessagePtr;
    
public:
    CallCenter(){}
    ~CallCenter(){}



    /**
     * @brief  yrpc实现长连接、session多协议复用功能的 core 函数，依靠Service函数实现协议服务端协议解析和协议分发
     * @param std::string_view(const)   入参 bytes 是 yrpc protocol 原始比特流
     * @return std::string 
     */
    static std::string Service(const std::string_view& bytes);


    
    /**
     * @brief 通过errcode 生成bytearray并返回
     * 
     * @param errcode 错误码
     * @return std::string 生成对应的错误响应报文
     */
    static int ErrCodeToByteArray(uint32_t id,uint32_t uid,int errcode,std::string& str,std::string);

};

} // namespace yrpc
