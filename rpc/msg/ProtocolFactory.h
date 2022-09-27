#pragma once

#include <unordered_map>
#include <functional>
#include <memory>
#include <initializer_list>
#include "../Util/noncopyable.h"
#include "../proto/c2s.pb.h"
#include "../proto/s2c.pb.h"
#include <google/protobuf/any.h>

namespace yrpc::rpc
{
typedef std::function<std::shared_ptr<google::protobuf::Message>()> CreateFunc;

#define YRPC_PROTO_REGISTER( ProtoID , ProtoType )  { ProtoID , []()->std::shared_ptr<google::protobuf::Message>{return std::make_shared<ProtoType>();}}

class ProtocolFactroy : yrpc::util::noncopyable::noncopyable
{
public:
    typedef uint16_t ProtoID;


    static ProtocolFactroy* GetInstance()
    {
        static ProtocolFactroy* m_instance = nullptr;
        if(m_instance == nullptr)
            m_instance = new ProtocolFactroy();
        return m_instance;
    }

    // ProtocolFactroy(){}

    // ProtocolFactroy(std::initializer_list<std::pair<ProtoID,CreateFunc>> args)
    // {
    //     Insert(std::move(args));
    // }


    void Insert(std::initializer_list<std::pair<ProtoID,CreateFunc>> args)
    {
        for(auto a: args)
        {
            m_cfactory.insert(a);
        }
    }


    std::shared_ptr<google::protobuf::Message> Create(ProtoID id)
    {
        auto it = m_cfactory.find(id);
        if(it == m_cfactory.end())
            return nullptr;
        else
            return it->second(); 
    }

    
private:
    std::unordered_map<ProtoID,CreateFunc> m_cfactory;
};


// void a()
// {
//     ProtocolFactroy::GetInstance()->Insert({
//         YRPC_PROTO_REGISTER(10001,C2S_HEARTBEAT_REQ)
//     });
// }


}