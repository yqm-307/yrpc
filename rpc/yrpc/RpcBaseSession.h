/**
 * @file RpcBaseSession.h
 * @author your name (you@domain.com)
 * @brief Session抽象
 * @version 0.1
 * @date 2022-09-16
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#pragma once
#include <memory>
#include <functional>
#include <condition_variable>
#include "../YRoutine/Hook.h"           // routine syscall hook 
#include "../protocol/ErrCode.h"        // errcode
#include "../network/Connector.h"       // tcp connection
#include "../Util/IDGenerate.h"         // id generate
#include "../network/SessionBuffer.h"   // buffer
#include "../protocol/RpcRequest.h"
#include "../protocol/RpcResponse.h"

namespace yrpc::rpc::detail
{

typedef std::shared_ptr<google::protobuf::Message> MessagePtr; 

class Base_RpcSession
{
public:

    Base_RpcSession(yrpc::detail::ynet::ConnectionPtr conn):m_conn(conn){}
    Base_RpcSession():m_conn(nullptr){}



    /* session是否已经建立连接，已经建立返回true，否则返回false */
    bool IsConnected()
    {
        if(m_conn != nullptr)
        {
            return !m_conn->IsClosed();
        }
        return false;
    }

    /**
     * @brief 发送SessionBuffer中所有数据。线程安全 
     */
    void SendAllByteArray()
    {
        std::string str{""};
        {
            yrpc::util::lock::lock_guard<yrpc::util::lock::Mutex> _lock(m_lock_out);
            m_output_buffer.GetByteArray(str);
        }
        if(!m_conn->IsClosed() && str.size() != 0)
        {
            int n = m_conn->send(str.c_str(),sizeof(str));
            DEBUG("RpcBaseSession::SendAllByteArray() , info: send %d bytes",n); 
        }
    }


protected:

    void AppendInputBuffer(const char* b,size_t len)
    { m_input_buffer.Save(b,len); }

    /* 如果 buffer 中有至少一个完整的包，将其填充至byte_array中。线程安全 */
    bool GetAProtocol(std::string& byte_array)
    {
        yrpc::util::lock::lock_guard<yrpc::util::lock::Mutex> _lock(m_lock_in);
        bool tmp;
        if(tmp=m_input_buffer.Has_Pkg())
            m_input_buffer.GetAReq(byte_array);
        return tmp; 
    }

    /* 如果 input buffer 中有一个完整的包，返回 true；否则返回 false。线程安全 */
    bool HasAProtocol()
    { 
        yrpc::util::lock::lock_guard<yrpc::util::lock::Mutex> _lock(m_lock_in);
        return m_input_buffer.Has_Pkg();
    }

    /* 将byte中取出长度为len的字节写入 outpur buffer 中 */
    void PendingToSend(const char* byte,size_t len)
    {
        yrpc::util::lock::lock_guard<yrpc::util::lock::Mutex> _lock(m_lock_out);
        m_output_buffer.Save(byte,len);
    }

    /* output buffer 字节数 */
    size_t OutLength()
    {
        return m_output_buffer.Length();
    }

    /* input buffer 字节数 */
    size_t InLength()
    {
        return m_input_buffer.Length();
    }



    yrpc::detail::ynet::ConnectionPtr m_conn;   // 
private:
    yrpc::detail::ynet::SessionBuffer m_input_buffer;   // input buffer
    yrpc::util::lock::Mutex m_lock_in;
    yrpc::detail::ynet::SessionBuffer m_output_buffer;  // output buffer
    yrpc::util::lock::Mutex m_lock_out;
};


}