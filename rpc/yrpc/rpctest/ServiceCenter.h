#include "../../proto/test_protocol/AddAndStr.pb.h"
#include "../YRpc.h"
#include <memory>
#include <bbt/timer/interval.hpp>
#include <bbt/config/GlobalConfig.hpp>
#include <string>
// #include "../../proto/test_protocol/AddAndStr.pb.h"

using namespace yrpc;
using namespace yrpc::util;
using namespace yrpc::util::clock;

typedef google::protobuf::Message Message;
typedef std::shared_ptr<Message> MessagePtr;
typedef std::function<void(std::shared_ptr<google::protobuf::Message>)> SendPacket;

class ServiceCenter
{
public:
    typedef uint64_t ServiceID;
    ServiceCenter();
    ~ServiceCenter();

    ServiceID AddressToID(const rpc::detail::Address& key)
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
    MessagePtr GetServerList(MessagePtr pck)
    {
        auto serv_list = std::make_shared<GetServiceListRsp>();
        for (auto&& i : m_map)
        {
            auto addr = serv_list->add_serv_list();
            addr->set_addr(i.second.GetIP()); 
            addr->set_port(i.second.GetPort());
        }
        return serv_list;
    }



private:
    std::map<ServiceID,yrpc::rpc::detail::Address> m_map;
};

ServiceCenter::ServiceCenter()
{
}

ServiceCenter::~ServiceCenter()
{
}
