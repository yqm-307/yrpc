#include "../network/SessionBuffer.h"

using namespace yrpc::detail::net;

std::string SessionBuffer::GetAReq()
{
    //包头
    int msglength = yrpc::util::protoutil::BytesToType<int>(buffer.peek());
    msglength = (uint16_t)(msglength);

    std::string reqbyte{""};
    reqbyte.resize(msglength);

    reqbyte.data();
    if (buffer.ReadableBytes() >= msglength)
    {
        buffer.ReadString(&*reqbyte.begin(), msglength); //将整个包读取并写入到reqbyte中
        return reqbyte;
    }
    else
        return "";
}

void SessionBuffer::GetAReq(std::string& bytearray)
{
    //包头
    int msglength = yrpc::util::protoutil::BytesToType<int>(buffer.peek());
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
    int msglength = yrpc::util::protoutil::BytesToType<int>(buffer.peek());
    if (buffer.ReadableBytes() < msglength) //如果可读字节数小于 msg 长度，说明包不完整，则返回true
        return true;
    else
        return false;
}


void SessionBuffer::GetByteArray(std::string& bytes)
{
    bytes.resize(buffer.ReadableBytes());
    buffer.ReadString(bytes.data(),buffer.ReadableBytes());
}
