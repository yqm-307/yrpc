/**
 *  想要做一个字节流处理，就是封装 rpc protocol 的一个 input/output 缓冲池
 *      1、需要能够添加和提取任意数量的字节数
 *      2、不需要验证，提供提取完整protocol接口，交给下一步验证即可
 *      3、底层使用 yrpc::util::buffer 实现
 *      4、不需要实现线程安全
 *      5、每个 connection 独有
 *      6、
 */
#pragma once
#include "../Util/Buffers.h"
#include "../protocol/proto.h"

namespace yrpc::detail::net
{


// 10月21日，回家速速重构


/**
 * @brief 一次会话中，只有一个buffer，可以缓存数据也可以拆分协议（解决粘包问题）
 */
class SessionBuffer
{
public:
    SessionBuffer(){}
    ~SessionBuffer(){}

    /**
     * @brief 添加 len 字节 data 到PackageManager
     * 
     * @param data 数据
     * @param len 长度
     * @return ssize_t 接收长度 
     */
    ssize_t Append(const char* data,size_t len)
    { buffer.WriteString(data,len); }


    /**
     * @brief 获取一个请求
     * 
     * @return std::string 请求的比特流 
     */
    std::string GetAPck();


    /**
     * @brief 获取一个请求
     * 
     * @param std::string& bytearray 请求的比特流
     */
    void GetAPck(std::string& bytearray);

    /**
     * @brief 是否有完整的pkg
     * 
     * @return true 
     * @return false 
     */
    bool Empty()
    { return buffer.DataSize() == 0;}

    /**
     * @brief 是否有一个完整的包
     * 
     * @return true 有一个完整的包
     * @return false 没有完整的包
     */
    bool Has_Pkg();

    void GetByteArray(std::string& byte);



    size_t Length()
    {
        return buffer.DataSize();
    }

private:
    yrpc::util::buffer::Buffer buffer;      // 
};

}



