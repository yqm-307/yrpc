#include "include/UdpAddress.h"
using namespace udp;

void initaddr(std::string&ip,int port,sockaddr_in& addr_)
{
    addr_.sin_family = AF_INET; 
    addr_.sin_port = htons(port);
    inet_pton(AF_INET,ip.c_str(),&addr_.sin_addr);
}

UdpAddress::UdpAddress(std::string& ip,int port)
    :ip_(ip),
    port_(port)
{
    initaddr(ip_,port_,addr_);
}    

UdpAddress::UdpAddress(std::string&& ip,int port)
    :ip_(ip),
    port_(port)
{
    initaddr(ip_,port_,addr_);
}

UdpAddress::UdpAddress(int ip_opt,int port)    //给监听使用
{
    addr_.sin_family = AF_INET;
    addr_.sin_addr.s_addr = htonl(INADDR_ANY);
    addr_.sin_port = htons(port);
}

void UdpAddress::setip(const std::string& ip)
{
    ip_ = ip;
    inet_pton(AF_INET,ip.c_str(),&addr_.sin_addr);
}

void UdpAddress::setport(int port)
{
    port_ = port;
    addr_.sin_port = htons(port_);
}