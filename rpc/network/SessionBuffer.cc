#include "../network/SessionBuffer.h"

using namespace yrpc::detail::net;

SessionBuffer::Buffer SessionBuffer::GetAPck()
{
    //包头
    auto msglength = yrpc::util::protoutil::BytesToType<uint16_t>(buffer.Peek());
    msglength = (uint16_t)(msglength);

    std::string reqbyte{""};
    reqbyte.resize(msglength);

    reqbyte.data();
    if (buffer.ReadableBytes() >= msglength)
    {
        buffer.ReadString(&*reqbyte.begin(), msglength); //将整个包读取并写入到reqbyte中
    }
    
    return std::move(reqbyte);
}

void SessionBuffer::GetAPck(std::string& bytearray)
{
    //包头
    int msglength = yrpc::util::protoutil::BytesToType<int>(buffer.Peek());
    msglength = (uint16_t)(msglength);

    std::string reqbyte{""};
    bytearray.resize(msglength);

    if (buffer.ReadableBytes() >= msglength)
        buffer.ReadString(bytearray.data(), msglength); //将整个包读取并写入到reqbyte中
    else
        bytearray.resize(0);
}


bool SessionBuffer::Has_Pkg()
{
    if ( buffer.DataSize() <= 0 )
        return false;
    // int msglength = yrpc::util::protoutil::BytesToType<int>(buffer.peek());  // 纯纯的nt,uint16_t 错当成 int
    uint16_t msglength = yrpc::util::protoutil::BytesToType<uint16_t>(buffer.Peek());
    if (buffer.ReadableBytes() <= msglength) //如果可读字节数小于 msg 长度，说明包不完整，则返回false
        return false;
    else
        return true;
}


void SessionBuffer::GetByteArray(std::string& bytes)
{
    bytes.resize(buffer.ReadableBytes());
    buffer.ReadString(bytes.data(),buffer.ReadableBytes());
}
