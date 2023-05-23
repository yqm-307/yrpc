/**
 * @file msgmap.h
 * @brief servicemap类维护一个全局单例的 {id,name,handler} 之间的映射
 * @version 0.1
 * @date 2022-06-12
 * @copyright Copyright (c) 2022
 */
#pragma once
#include <unordered_map>
#include <tuple>
#include <algorithm>
#include <any>
#include <vector>
#include <iostream>
#include <google/protobuf/any.h>
#include "../Util/noncopyable.h"

namespace yrpc::detail
{


enum CodeOrService : uint32_t
{
    EnCode = 0, // 将arg1(Message*)序列化到arg2(string)
    Decode = 1, // arg2(Message)是解析结果，arg1(string)是参数
};

typedef std::shared_ptr<google::protobuf::Message> ProtoMsgPtr; // std::shared_ptr<protobuf::message*>  -- 包装裸指针

typedef std::function<void(ProtoMsgPtr)>    SendPacketFunc;

typedef std::function<void(bool, ProtoMsgPtr &, std::string &)> CodecFunc;    // typedef std::function<void(bool, std::any &, std::any &)>  -- codec函数
typedef std::function<void(const ProtoMsgPtr/*req*/,SendPacketFunc&&)> ServiceFunc; // std::function<google::protobuf::Message*(const std::any /*req*/)>  -- 服务处理函数

typedef std::pair<ServiceFunc, CodecFunc> ServiceHandles;       // std::pair<ServiceFunc, CodecFunc>    -- service handler函数和codec函数



class ServiceMap:bbt::noncopyable
{
public:
    ServiceMap();
    ~ServiceMap();

    /**
     * @brief 获取单例对象
     * 
     * @return ServiceMap* ServiceMap指针 
     */
    static ServiceMap* GetInstance()
    {
        static ServiceMap* instance_ = nullptr;
        if(instance_ == nullptr)    
            instance_ = new ServiceMap;
        return instance_;
    }


    /**
     * @brief 添加一个service
     * 
     * @param name service name
     * @param service service handlers
     * @param code service codec handlers
     * @return uint32_t 返回 service id，service id 等于 0 说明错误
     */
    uint32_t insert(std::string name, const ServiceFunc& service ,const CodecFunc& code);

    uint32_t insert(std::string name, uint32_t id,const ServiceFunc& service ,const CodecFunc& code);
    

    /**
     * @brief 根据 id 获取删除指定service
     * 
     * @param id service id
     * @return int 成功返回0，小于0失败
     */
    int earse(int id);


    /**
     * @brief 根据id获取handler
     * 
     * @param id service id
     * @return const ServiceHandles* service handler 
     */
    const ServiceHandles* IdToService(uint32_t id) const;
    
    
    /**
     * @brief 获取两个函数，第一个是服务函数，第二个是解析函数
     * 
     * @param name 
     * @return const ServiceHandles* 第一个函数模型：std::function<std::string(ArgsPtr&)> 
     *  第二个函数模型std::function<std::any(const std::string_view&)>
     */
    const ServiceHandles* NameToService(std::string name) const;
private:
    
    typedef std::unordered_map<uint32_t,ServiceHandles*> ITSMap; 
    typedef std::unordered_map<std::string,uint32_t> NTIMap;
    typedef std::unordered_map<uint32_t,std::string> ITNMap;


    /* 维护 id、name、handler 之间的映射关系 */


    NTIMap NameToId_;       //服务名 到 服务的映射
    ITSMap IdToService_;    //id 到 服务的映射
    ITNMap IdToName_;       //id 到 服务名
};


}





