//
#pragma once
#include "../Util/Locker.h"
#include "RpcBaseSession.h"
#include "../msg/ProtocolFactory.h"
#include <memory>
#include <functional>
#include <condition_variable>
#include <unordered_map>
#include <map>
#include <set>


namespace yrpc::rpc::detail
{


class RpcClientSession;
typedef std::shared_ptr<google::protobuf::Message> MessagePtr;
typedef std::function<void(MessagePtr)> RpcCallback;

class Future
{
    friend class RpcClientSession;
public:


public:

    Future(){}
    Future(const Future& p)
            :m_send(p.m_send),
            m_recv(p.m_recv),
            // m_session(p.m_session),
            m_func(p.m_func)
    {}
    ~Future(){}

    void Get(MessagePtr proto)
    {
        m_sem.wait();
        proto = m_recv;
    }
    bool TryGet(int interval_ms){}

protected:
    Future(std::shared_ptr<google::protobuf::Message> s)
            :m_send(s),
            m_recv(nullptr)
            // m_session(session)
    {}
    Future(std::shared_ptr<google::protobuf::Message> s,RpcCallback func)
            :m_send(s),
            m_recv(nullptr),
            m_func(func)
            // m_session(session)
    {}


    bool IsFinish()
    { return (m_recv!=nullptr); }
    bool IsAsync()
    { return (m_func!=nullptr); }
    bool Async()
    {
        if(m_func == nullptr)
            return false;
        m_func(m_recv);
        return true;
    }   
    void SetResult(MessagePtr result)
    {
        if(result == nullptr)
            return;
        if(m_func == nullptr)
        {               
            m_recv = result;
            m_sem.notify_all();
        }
        else
        {
            m_func(result);
        }
    }

private:
    MessagePtr m_send;
    MessagePtr m_recv;
    RpcCallback /*void(std::shared_ptr<google::protobuf::message*>)*/  m_func;
    // std::shared_ptr<RpcClientSession> m_session;
    yrpc::util::lock::Mutex m_mutex;
    yrpc::util::lock::Sem_t m_sem;
};



}