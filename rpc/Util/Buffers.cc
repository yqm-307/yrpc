#include <assert.h>
#include <sys/uio.h>    //散步读和聚集写
#include "./Buffers.h"
#include "./logger.h"

namespace yrpc::util::buffer
{

const int HeaderBytes=0;        // 预留位置
const int Buffer::headSize=0;   // 
const int Buffer::initSize=4096;// 初始长度


Buffer::Buffer(size_t size)
    :bytes(size),
    _readIndex(headSize),
    _writeIndex(headSize),
    reservedBytes(HeaderBytes)
{}

Buffer::Buffer(const Buffer& rval)
    :bytes(rval.bytes),
    reservedBytes(HeaderBytes),
    _readIndex(rval._readIndex),
    _writeIndex(rval._writeIndex)
{}

Buffer::Buffer(Buffer&& rval)
    :reservedBytes(HeaderBytes),
    _readIndex(rval._readIndex),
    _writeIndex(rval._writeIndex)
{
    bytes = std::move(rval.bytes);
    rval.InitAll();
}




//可读 = 已写 - 已读
size_t Buffer::ReadableBytes() const
{
    return _writeIndex - _readIndex;
}

//可写字节数
size_t Buffer::WriteableBytes() const
{
    return bytes.size() - _writeIndex;
}

//前置空闲字节数
size_t Buffer::PrepareBytes() const
{
    return _readIndex;
}

//swap
void Buffer::swap(Buffer& s)
{
    bytes.swap(s.bytes);
    std::swap(_readIndex, s._readIndex);
    std::swap(_writeIndex, s._writeIndex);
}

void Buffer::InitAll()                 //初始化
{
    _writeIndex = headSize;
    _readIndex = headSize;
}

//读取len个字节到byte中
bool Buffer::Read(void* byte, int len)
{
    assert(ReadableBytes() >= len);
    memcpy(byte,begin()+_readIndex,len);
    _readIndex+=len;
    return true;
}


//start位置长度为len的内存，移动到obj处
void Buffer::move(int obj, int src, int len)
{
    memcpy(begin() + obj, begin() + src, len);
}

//将数据移动到前方
void Buffer::moveForward()
{
    if (_readIndex == headSize)
        return;
    int buffsize = ReadableBytes();
    move(headSize, _readIndex, ReadableBytes());
    _readIndex = headSize;
    _writeIndex = headSize+buffsize;
}


bool Buffer::Write(const char* data, size_t len)
{
    if (WriteableBytes() < len)  //可写空间不足
    {
        moveForward();  //向前移动
        
        //尽量写
        int shengyu = WriteableBytes();
        if(len > WriteableBytes())
            memcpy(begin() + _writeIndex, data, shengyu);
        else
            memcpy(begin() + _writeIndex, data, len);
        int lenshengyu = len - shengyu;
        
        if(lenshengyu > 0)
        //扩容
        for (int i = shengyu; i < len; ++i)
        {
            bytes.push_back(*(data + i));
        }
        _writeIndex += len;
            
    }
    else    //可写空间足够
    {
        memcpy(begin() + _writeIndex, data, len);
        _writeIndex += len;
    }
    
    
    return true;

}



bool Buffer::WriteInt64(int64_t num)
{
    char buf[8];
    memcpy(buf,&num,8);
    return Write(buf,sizeof(buf));
}
bool Buffer::WriteInt32(int32_t num)
{
    char buf[4];
    memcpy(buf,&num,4);
    return Write(buf,sizeof(buf));
}
bool Buffer::WriteInt16(int16_t num)
{
    char buf[2];
    memcpy(buf,&num,2);
    return Write(buf,sizeof(buf));
}
bool Buffer::WriteInt8(int8_t num)
{
    char buf[1];
    memcpy(buf,&num,1);
    return Write(buf,sizeof(buf));
}
bool Buffer::WriteString(std::string str)
{
    char buf[str.size()];
    memcpy(buf,str.c_str(),str.size());
    return Write(buf,sizeof(buf));
}
bool Buffer::WriteString(const char* p , int len)
{
    char buf[len];
    memcpy(buf,p,len);
    return Write(buf,len);
}

int64_t Buffer::ReadInt64()
{
    assert(ReadableBytes()>=sizeof(int64_t));
    int64_t ret;
    Read(&ret,sizeof(int64_t));
    return ret;
}
int32_t Buffer::ReadInt32()
{
    assert(ReadableBytes()>=sizeof( int32_t));

    int32_t ret;
    Read(&ret,sizeof(int32_t));
    return ret;
}
int16_t Buffer::ReadInt16()
{
    assert(ReadableBytes()>=sizeof(int16_t));
    int16_t ret;
    Read(&ret,sizeof(int16_t));
    return ret;    
}
int8_t  Buffer::ReadInt8()
{
    assert(ReadableBytes()>=sizeof(int8_t));
    int8_t ret;
    Read(&ret,sizeof(int8_t));
    return ret;
}
void Buffer::ReadString(std::string& ret,int len)
{
    assert(ReadableBytes()>=len);
    char buf[len];
    Read(buf,len);
    ret = std::string(buf);
}

void Buffer::ReadString(char* ret,int len)
{
    assert(ReadableBytes()>=len);
    Read(ret,len);
}


/*
    将数据读到vec0和vec1中，如果 inputbuffer 满了，就先暂存到缓冲区。
    之后判断有没有用到缓冲区，用到了就移动到inputbuffer中
*/
int64_t Buffer::readfd(int fd, int& savedErrno)
{
    char extrabuf[65536];
    struct iovec vec[2];
    const size_t writable = WriteableBytes();   //缓冲区可写字节数
    vec[0].iov_base = begin() + _writeIndex;
    vec[0].iov_len = writable;
    vec[1].iov_base = extrabuf;
    vec[1].iov_len = sizeof extrabuf;

    //是否用得到缓冲区呢
    const int iovcnt = (writable < sizeof extrabuf) ? 2 : 1;
    const ssize_t n = ::readv(fd, vec, iovcnt);

    if (n < 0)
        savedErrno = errno;
    else if (static_cast<size_t>(n) <= writable)
        _writeIndex += n;
    else {
        _writeIndex = bytes.size();
        WriteString(extrabuf, n - writable);
    }
    return n;
}



const char* Buffer::peek() const
{
    return peek();
}
char* Buffer::peek()
{
    return begin()+_readIndex;
}


void Buffer::recycle(size_t n) //回收n字节空间
{
    if(n<ReadableBytes())
    {
        _readIndex+=n;
    }
    else
        InitAll(); //初始化整个空间
}

}
