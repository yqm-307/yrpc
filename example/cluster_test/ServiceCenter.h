#pragma once
#include "../all_example.pb.h"
#include "yrpc/core/YRpc.h"
#include "yrpc/core/Define.h"
#include <memory>
#include <bbt/timer/interval.hpp>
#include <bbt/config/GlobalConfig.hpp>
#include <string>

using namespace yrpc;
using namespace yrpc::util;
using namespace yrpc::util::clock;

typedef google::protobuf::Message Message;
typedef std::shared_ptr<Message> MessagePtr;
typedef yrpc::detail::net::YAddress Address;
typedef std::function<void(std::shared_ptr<google::protobuf::Message>)> SendPacket;

class ServiceCenter
{
public:
    typedef uint64_t ServiceID;
    ServiceCenter();
    ~ServiceCenter();

    ServiceID AddressToID(const Address& key)
    {
        auto str = key.GetIPPort();
        std::string id(19, '0');
        int j = 0;
        for (int i = 0; i < str.size(); ++i)
        {
            if (str[i] >= '0' && str[i] <= '9')
            {
                id[j++] = str[i];
            }
        }
        return std::stoull(id);
    }
private:
    /* 获取服务器列表 */
    MessagePtr GetServerList(MessagePtr pck);
    /* 注册服务器地址 */
    MessagePtr RegistServer(MessagePtr pck);
    MessagePtr HeartBeat(MessagePtr pck);

private:
    void DelAddr(ServiceID id);
    void AddAddr(ServiceID id, Address addr);

private:
    std::map<ServiceID, Address> m_map;
};

ServiceCenter::ServiceCenter()
{
}

ServiceCenter::~ServiceCenter()
{
}
