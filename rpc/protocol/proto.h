/**
 * @file proto.h
 * @author yqm-307 (979336542@qq.com)
 * @brief protobuf Utils 提供一些通用的protobuf工具
 * @version 0.1
 * @date 2022-06-11
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#pragma once
#include <google/protobuf/message.h>
#include <any>
#include <memory>
#include "../Util/BKDRHash.h"


namespace yrpc::util::protoutil
{
typedef std::vector<std::any> Argspkg;
typedef std::shared_ptr<Argspkg> ArgsPtr;
typedef google::protobuf::Message Message;
typedef google::protobuf::Descriptor Descriptor;
typedef google::protobuf::FieldDescriptor FieldDescriptor;
typedef google::protobuf::Reflection Reflection;

//c风格接口创建和销毁Argspkg
Argspkg* InitArgs();
void DestoryArgs(Argspkg* args);



//解析一个msg并保存所有值在一个std::vector<std::any>容器中。
ArgsPtr ParseProtoMsg(const google::protobuf::Message& message);

//返回一个 int 或者 vector<int>
std::any ParseInt32(const Message& message,const Reflection* ref,const FieldDescriptor* field);

//返回一个 int64 或者 vector<int64>
std::any ParseInt64(const Message& message,const Reflection* ref,const FieldDescriptor* field);

//返回一个 uint32 或者 vector<uint32>
std::any ParseUInt32(const Message& message,const Reflection* ref,const FieldDescriptor* field);

//返回一个uint64 或者 vector<uint64>
std::any ParseUInt64(const Message& message,const Reflection* ref,const FieldDescriptor* field);

//返回一个 double 或者 vector<double>
std::any ParseDouble(const Message& message,const Reflection* ref,const FieldDescriptor* field);

//返回一个 float 或者 vcetor<float>
std::any ParseFloat(const Message& message,const Reflection* ref,const FieldDescriptor* field);

//返回一个 bool 或者 vector<bool>
std::any ParseBool(const Message& message,const Reflection* ref,const FieldDescriptor* field);

//返回一个 EnumValeDescriptor* 或者 vector<>
std::any ParseEnum(const Message& message,const Reflection* ref,const FieldDescriptor* field);

//返回一个 string 或者 vector<string>
std::any ParseString(const Message& message,const Reflection* ref,const FieldDescriptor* field);

//返回一个 ArgsPtr
std::any ParseMessage(const Message& message,const Reflection* ref,const FieldDescriptor* field);



/**
 * @brief 将字节流转化为 T 类型变量(不安全操作，隐患:data随机内存访问)
 * 
 * @tparam T 
 * @param data 字节指针
 * @return T 
 */
template<typename T>
T BytesToType(const char* data)
{
    T tmp;
    memcpy(&tmp,data,sizeof(T));
    return tmp;
}


/**
 * @brief 将data值，逐字节拷贝到src中(不安全操作，隐患:src导致随机内存访问)
 * 
 * @tparam T 
 * @param data 
 * @param src 
 */
template<typename T>
void ToBytes(T data ,char* src)
{
    memcpy(src,(char*)(&data),sizeof(T));
}





// //将src中len长度的字节，追加到dest里面
// void AppendToString(std::string& dest,const char* src,size_t len)
// {
//     dest.append(src,len);
// }



}