#pragma once
#include <iostream>
#include <netinet/in.h>
#include <sys/socket.h>

namespace yrpc::detail::net
{

class IPAddress
{
public:
    IPAddress(std::string ip,int port);
    //给服务器初始化使用
    explicit
    IPAddress(int port, int opt = INADDR_ANY);
    IPAddress() = default;
    ~IPAddress(){};

    void set(std::string ip,int port);
    void set(sockaddr_in addr);
    std::string GetIP() const;
    int GetPort() const;
    const struct sockaddr* getsockaddr() const 
    {   return reinterpret_cast<const sockaddr*>(&_addr);}
    const socklen_t getsocklen() const
    {   return sizeof(_addr);}
    std::string GetIPPort() const;
    //char* StringToCstr(std::string ip);
protected:
    struct sockaddr_in _addr;
    std::string _ip;
    int _port;
};



class Socket;

class YAddress final:public IPAddress 
{
public:
    YAddress(std::string ip,int Port)
        :IPAddress(ip,Port),is_null(false)
    {

    }
    YAddress(int port,int opt = INADDR_ANY)
        :IPAddress(port,opt),is_null(false)
    {

    }
    YAddress():is_null(true){}
    explicit operator bool() const
    { return is_null;}

    bool operator==(const YAddress& oth)
    {
        return  ((this->_ip==oth._ip) && (this->_port == oth._port));
    }

    bool operator!=(const YAddress& oth)
    {
        return !(*this == oth);
    }
    


private:
    bool is_null;
};

}