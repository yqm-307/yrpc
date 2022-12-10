/**
 * @file Codec.h
 * @author yqm-307 (979336542@qq.com)
 * @brief 
 * @version 0.1
 * @date 2022-06-12
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#pragma once
#include <google/protobuf/any.h>
#include <functional>
#include <iostream>
#include <string>
#include "proto.h"




namespace yrpc::detail
{
typedef yrpc::util::protoutil::Argspkg Argspkg;
typedef yrpc::util::protoutil::ArgsPtr ArgsPtr; 

class YRPCProtocol;

/*
*   protobuf序列化和反序列化工具
*/
class Codec
{
    // Codec(){}
    // virtual ~Codec(){}
public:


    /**
     * @brief 将 bytes 转化为 一个 参数对象，并以智能指针的形式传出
     * 
     * @tparam MsgType 传入bytes将要转换对象的类型
     * @param bytes_ 待转换数据字节流
     * @return ArgsPtr ArgsPkg的智能指针形式
     */
    template<class MsgType>
    static ArgsPtr Parse(const std::string_view& bytes_)
    {
        MsgType msgtype;
        google::protobuf::Message& msg = msgtype;
        msg.ParseFromArray(bytes_.data(),bytes_.size());
        auto args = yrpc::util::protoutil::ParseProtoMsg(msg);
        
        return args;
    }

    /**
     * @brief 将bytes转化为一个MsgType类型的只智能指针对象
     * 
     * @tparam MsgType 
     * @param std::string bytes 
     * @return std::shared_ptr<MsgType> 
     */
    template<class MsgType>
    static std::shared_ptr<MsgType> ParseToMessage(const std::string_view& bytes)
    {   
        // google::protobuf::Message* msgptr;
        // auto msgptr = std::make_shared<google::protobuf::Message>();
        auto msgptr = std::make_shared<MsgType>();
        msgptr->ParseFromArray(bytes.data(),bytes.size());
        return msgptr;
    }

    static bool ParseToMessage(std::shared_ptr<google::protobuf::Message> msgptr,const std::string& bytes)
    {
        return msgptr->ParseFromArray(bytes.data(),bytes.size());
    }
    static bool ParseToMessage(std::shared_ptr<google::protobuf::Message> msgptr,const std::string_view& bytes)
    {
        return msgptr->ParseFromArray(bytes.data(),bytes.size());
    }

    /**
     * @brief 将msg序列化数据输入到bytes
     * 
     * @tparam MsgType 协议类型
     * @param google::protobuf::Message* msg 
     * @param std::string bytes  
     * @return bool 成功or失败 
     */
    template<class MsgType>
    static bool Serialize(google::protobuf::Message* msg,std::string& bytes)
    {
        MsgType* t = reinterpret_cast<MsgType*>(msg);
        return t->AppendToString(&bytes);
    }

    static bool Serialize(std::shared_ptr<google::protobuf::Message> msg,std::string& bytes)
    {
        // std::shared_ptr<MsgType> t = static_cast<std::shared_ptr<MsgType>>(msg);
        return msg->AppendToString(&bytes);
    }


};





}

