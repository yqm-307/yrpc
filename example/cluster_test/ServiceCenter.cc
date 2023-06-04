#include "ServiceCenter.h"


MessagePtr ServiceCenter::GetServerList(MessagePtr pck)
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

MessagePtr ServiceCenter::RegistServer(MessagePtr pck)
{
    auto req = std::static_pointer_cast<ServiceRegistReq>(pck);
    auto rsp = std::make_shared<ServiceRegistRsp>();
    std::string ip = req->addr().addr();
    int port = req->addr().port();
 
    Address addr(ip, port);
    auto sid = this->AddressToID(addr);
    auto it = m_map.find(sid);
    if (it != m_map.end())
    {
        rsp->set_status(2);
        rsp->set_msg("address repeat");
    }
    else
    {
        m_map.insert(std::make_pair(sid,addr));
        rsp->set_status(1);
        rsp->set_msg("success");
    }
    return rsp;
}