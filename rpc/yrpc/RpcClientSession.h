/**
 * @file RpcClientSession.h
 * @author your name (you@domain.com)
 * @brief client 端的 rpc session
 * @version 0.1
 * @date 2022-09-16
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#pragma once

#include "RpcFuture.h"

namespace yrpc::rpc::detail
{


class RpcClientSession final : public Base_RpcSession , std::enable_shared_from_this<RpcClientSession>
{
public:
    // RpcClientSession(yrpc::coroutine::poller::Epoller* context)
    //         :m_io_context(context)
    // {
    // }
    friend class Future;
    typedef std::shared_ptr<RpcClientSession> Ptr;
    
    RpcClientSession(yrpc::coroutine::poller::Epoller* context,yrpc::detail::ynet::ConnectionPtr conn)
            :Base_RpcSession(conn),
            m_io_context(context),
            m_now_tasknum(0)
    {
        m_io_context->AddTask([this](void*){Handler();},nullptr);
        m_conn->setOnRecvCallback([this](const char* buff,size_t len){input_routinue(buff,len);});
        m_conn->update();  
    }
    ~RpcClientSession()
    {
        if(!m_conn->IsClosed())   
            m_conn->Close();
    }

    /**
     * @brief 注册一个异步调用，将proto和func绑定传递给RpcClient核心。
     * 
     * @param std::shared_ptr<google::protobuf::Message> proto 
     * @param void(std::shared_ptr<google::protobuf::Message>) func 
     * @return true 注册成功
     * @return false 注册失败
     */
    bool RpcAsyncCall(std::shared_ptr<google::protobuf::Message> proto,RpcCallback func);

    /**
     * @brief 注册一个 Future 异步调用，类似同步模式异步调用
     * 
     * @param Future& future 
     * @return true 注册成功
     * @return false 注册失败
     */
    bool RpcSyncCall(Future& future);

    Ptr GetShared()
    {
        return shared_from_this();
    }

private:
    void Handler();
    /* 接收缓冲数据到SessionBuffer中，不做处理*/
    void input_routinue(const char* buffer,size_t len);
    /* 发送协程，只要 output buffer 中数据达到一定量就唤醒该循环，send 数据 */
    void output_routinue();
    void Dispatch(const yrpc::detail::protocol::RpcResponse&);
    
private:
    void S2C_RPC_CALL_RSP_Handler(const yrpc::detail::protocol::RpcResponse&);
    void S2C_HEARTBEAT_RSP_Handler(const yrpc::detail::protocol::RpcResponse&);

private:
    typedef uint32_t ProtoID; 
    typedef std::pair<ProtoID,Future*> Entry;
    typedef std::queue<Entry> RegisterQueue;
    typedef std::unordered_map<ProtoID,Future*> RegisteredMap;   // 已注册任务
    typedef std::unordered_map<ProtoID,Future*> FinishedMap;     // 已回复任务
    typedef std::priority_queue<uint32_t>  TimeQueue;            // 超时队列
    typedef std::unordered_set<uint32_t> RegisteredProtoIDSet;   // 已经注册ProtoID集合

    yrpc::coroutine::poller::Epoller* m_io_context;
    
    RegisterQueue m_registerqueue;
    yrpc::util::lock::Mutex m_regiseer_lock;
    int max_size;
    RegisteredMap m_regiseredmap;
    FinishedMap m_finishedmap;
    yrpc::util::lock::Mutex m_finish_lock;
    const int m_max_task{128};
    std::atomic_int m_now_tasknum;


    // 超时队列
    yrpc::util::clock::YTimer<ProtoID> m_timequeue;

    yrpc::socket::Epoll_Cond_t m_handler_cond;

};


}