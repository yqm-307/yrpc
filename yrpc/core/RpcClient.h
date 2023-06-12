/**
 * @file RpcClient.h
 * @author yqm-307 (979336542@qq.com)
 * @brief RpcClient 是YRPC 中最常用的类之一，通过该类才可以发起调用
 * @version 0.1
 * @date 2022-06-20
 * 
 * @copyright Copyright (c) 2022
 */
#pragma once
#include "./Define.h"
#include "./CallObjFactory.h"

namespace yrpc::rpc
{


/**
 * @brief RpcClient 是调用方最重要的类，通过该类才可以发起调用
 */
class RpcClient
{
    // friend class detail::CallObj;
    typedef yrpc::util::lock::Mutex                         Mutex;
    typedef std::shared_ptr<detail::RpcSession>             SessionPtr;
    typedef google::protobuf::Message                       Message;
    typedef std::map<Protocol_PckIdType,detail::CallObj::Ptr> CallObjMap;      
    typedef yrpc::util::buffer::Buffer                      Buffer;
    typedef yrpc::detail::protocol::YProtocolGenerater  Generater;  // 存储 request 并提供序列化
    typedef yrpc::detail::protocol::YProtocolResolver   Resolver;   // 存储 response bytearray 提供反序列化
public:

    /**
     * @brief 构造一个RpcClient
     * 
     * @param ip    服务提供方IP地址
     * @param port  服务提供方端口
     * @param logpath       日志名
     * @param stack_size    协程栈大小，默认128kb
     * @param maxqueue      协程数上限
     */
    RpcClient(std::string ip,int port);
    RpcClient(detail::Address servaddr_);
    ~RpcClient();   //todo,退出不安全，在Client关闭时，应该先让所有请求都返回或者直接失败    
    /**
     * @brief 是否已经建立连接
     * 
     * @return true  已经完成连接
     * @return false 连接尚未完成
     */
    bool IsConnected();
    /**
     * @brief 发起一次调用
     * 
     * @param call 调用
     * @return int 
     */
    int Call(detail::CallObj::Ptr call);
    /**
     * @brief 发起一次异步Connect，连接对端成功，将会在当前线程调用 
     * 
     * @param func 
     */
    void AsyncConnect(detail::OnConnCallBack func);
private:
    /**
     * @brief 连接建立完成，通过newsession返回session
     * 
     * @param newsession 
     */
    void OnConnect(SessionPtr newsession);

    /**
     * @brief 此函数处理一条协议，并通知调用者
     * 
     * @param pck 一条协议的字节流
     */
    void OnPckHandler(Buffer&& pck);

private:
    SessionPtr          m_session;          // 实现rpc IO 操作
    detail::Address     m_addr;
    CallObjMap          m_callmap;          // map
    Mutex               m_mutex;

}; // RpcClient
}// namespace yrpc::rpc

