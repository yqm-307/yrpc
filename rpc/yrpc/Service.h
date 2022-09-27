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
     * @brief 核心函数，传入一个以定义的协议，通过已注册的协议处理程序处理协议，并将结果序列化并返回
     * 
     * @param bytes recv 协议的原始字节流数据 
     * @return std::string 处理后协议响应的字节流数据
     */
    std::string Service(const std::string_view& bytes);


private:
    

    /**
     * @brief 通过errcode 生成bytearray并返回
     * 
     * @param errcode 错误码
     * @return std::string 生成对应的错误响应报文
     */
    int ErrCodeToByteArray(uint32_t id,uint64_t uid,int errcode,std::string& str);
    

private:
    yrpc::coroutine::poller::Epoller* scheduler_;
};

} // namespace yrpc
