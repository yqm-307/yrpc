#pragma once
#include "../network/TcpServer.h"
#include "../msg/servicemap.h"
#include "../protocol/ProtocolUtil.h"
#include "../Util/all.h"
#include "../protocol/ErrCode.h"


namespace yrpc::rpc::detail
{
class CallCenter
{
public:
    CallCenter(){}
    ~CallCenter(){}


    /**
     * @brief  yrpc实现长连接、session多协议复用功能的 core 函数，依靠Service函数实现协议服务端协议解析和协议分发
     * @param std::string_view(const)   入参 bytes 是 yrpc protocol 原始比特流
     * @return std::string 
     */
    std::string Service(const std::string_view& bytes);


private:
    

    /**
     * @brief 通过errcode 生成bytearray并返回
     * 
     * @param errcode 错误码
     * @return std::string 生成对应的错误响应报文
     */
    int ErrCodeToByteArray(uint32_t id,uint32_t uid,int errcode,std::string& str,std::string);
    

private:
    yrpc::coroutine::poller::Epoller* scheduler_;
};

} // namespace yrpc
