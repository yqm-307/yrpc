#ifndef UDPADDRESS_H
#define UDPADDRESS_H

#include <iostream>
#include <string>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>  //字节序转换

namespace udp
{
class UdpAddress
{
public:
    UdpAddress()=default;
    UdpAddress(std::string& ip,int port);    
    UdpAddress(std::string&& ip,int port);
    UdpAddress(int ip_opt,int port);    //给监听使用
    ~UdpAddress();

    sockaddr* getsockaddr()
    { return reinterpret_cast<sockaddr*>(&addr_);}

    const std::string& getIP() const 
    { return ip_; }
    int getPort() const
    {return port_;}
    std::string getIPPortstr() const
    { return ip_+ ":"+std::to_string(port_);}


    void setip(const std::string& str);
    void setport(int port);

private:
    std::string ip_;
    int port_;
    sockaddr_in addr_;
};
}
#endif