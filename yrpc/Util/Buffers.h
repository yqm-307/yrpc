#pragma once
#include <iostream>
#include <algorithm>
#include <vector>
#include <string>
#include <cassert>
#include <cstring>
#include "./logger.h"

namespace yrpc::util::buffer
{



class Buffer
{
public:
    static const int headSize;  //可以插入消息id或者长度
    static const int initSize;
    Buffer(size_t initsize = initSize);
    ~Buffer(){
        // DEBUG("this: 0x%x  bytes:0x%x\t",this,&*bytes.begin());
    }

    Buffer(const Buffer& rval);
    Buffer(Buffer&& rval);
    Buffer(const char* bytes , size_t len);
    Buffer(const std::string& bytes);
    // Buffer(std::string&& bytes);

    Buffer& operator=(Buffer&&);
    Buffer& operator=(const Buffer&);


    //常见值的读写
    /* 与buffer交换数据 */
    void Swap(Buffer& buffer);          //拷贝一个buffer
    /* 初始化为原始状态 */
    void InitAll();                 //初始化
    /* 将num作为比特流写入buffer */
    bool WriteInt64(int64_t num);
    /* 将num作为比特流写入buffer */
    bool WriteInt32(int32_t num);
    /* 将num作为比特流写入buffer */
    bool WriteInt16(int16_t num);
    /* 将num作为比特流写入buffer */
    bool WriteInt8(int8_t num);
    /* 将str作为字节流写入 buffer */
    bool WriteString(std::string str);
    /* 将str作为字节流写入 buffer*/
    bool WriteString(const char* p ,size_t len);

    /* 从buffer中读取 int64  */
    int64_t ReadInt64();
    /* 从buffer中读取 int32  */
    int32_t ReadInt32();
    /* 从buffer中读取 int16  */
    int16_t ReadInt16();
    /* 从buffer中读取 int8  */
    int8_t  ReadInt8();
    /* 从buffer读取长度为 len 的字节流，并写入 str */
    void ReadString(std::string& str,size_t len);
    /* 从buffer读取长度为 len 的字节流，并写入 str */
    void ReadString(char* str,size_t len);

    /* 返回当前数据段第一个字节的地址 */
    const char* Peek(size_t n=0) const;
    char* Peek(size_t n=0);
    /* 丢弃 n 字节未读数据，如果未读数据小于 n，初始化整个buffer */
    void Recycle(size_t n);                 
    /* 返回可读字节数 */
    size_t ReadableBytes() const;               
    /* 返回buffer当前可写字节数 */
    size_t WriteableBytes() const;              
    /* 返回当前peek的前置空间 */
    size_t PrepareBytes() const;                 
    /* buffer 数据段长度 */
    size_t DataSize() const {return ReadableBytes();}  //buffer 数据段长度
    /* 获取为std::string_view */
    std::string_view View() const
    { return std::string_view(Begin(),ReadableBytes()); }
    /* 从sockfd 读取字节流到buffer中 */
    int64_t Readfd(int sockfd,int& Errno);  //connector接受数据使用
    /* 获取当前可读数据首 */

    /* 写入 n 个随机字符，本质上就是扩充 */
    size_t WriteNull(size_t n);

private:   
    bool Read(void* ,size_t len);                         
    bool Write(const char* data, size_t len);    
    char* GetOffset(size_t n);
    const char* GetOffset(size_t n) const;
    
    char* Begin()             
    {return &*bytes.begin();}
    const char* Begin() const                   
    {return &*bytes.begin();}
    void move(int start,int len,int obj);       //移动
    void moveForward();                         //向前移动
private:
    std::vector<char> bytes;            //比特流
    size_t _readIndex{0};                  //已读
    size_t _writeIndex{0};                 //已写
    const int reservedBytes{0};            //预留位置

    const char CRLF[3] = "\r\n";
};


}
