#pragma once
#include <unordered_map>
#include <functional>
#include <memory>
#include <initializer_list>
#include "../Util/noncopyable.h"
#include "../Util/Assert.h"
#include "yrpc/protocol/protoc/c2s.pb.h"
#include "yrpc/protocol/protoc/s2c.pb.h"
#include <google/protobuf/any.h>

namespace yrpc::rpc
{
typedef std::function<std::shared_ptr<google::protobuf::Message>()> CreateFunc;

#define YRPC_PROTO_REGISTER( ProtoID , ProtoType )  { ProtoID , \
    []()->std::shared_ptr<google::protobuf::Message> \
    { \
        return std::make_shared<ProtoType>(); \
    } \
}






class ProtocolFactroy : bbt::noncopyable
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

    bool Insert(std::initializer_list<std::pair<ProtoID,CreateFunc>> args)
    {
        bool ret{true};
        for(auto a: args)
        {
            auto result = m_cfactory.insert(a);
            if (!result.second)
            {
                ret = false;
                break;
            }
        }
        return ret;
    }

    bool Insert(std::pair<ProtoID,CreateFunc>& pair)
    {
        auto result = m_cfactory.insert(pair);
        return result.second;
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




}