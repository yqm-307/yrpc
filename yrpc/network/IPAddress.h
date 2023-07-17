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
    virtual ~IPAddress(){};
    IPAddress(IPAddress&&);
    IPAddress(const IPAddress&);

public:
    void set(std::string ip,int port);
    void set(sockaddr_in addr);
    virtual std::string GetIP() const;
    virtual int GetPort() const;
    const struct sockaddr* getsockaddr() const 
    {   return reinterpret_cast<const sockaddr*>(&m_addr);}
    const socklen_t getsocklen() const
    {   return sizeof(m_addr);}
    std::string GetIPPort() const;
protected:
    struct sockaddr_in m_addr;
    std::string m_ip;
    int         m_port;
};




class YAddress final : public IPAddress 
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

    YAddress(YAddress&&);
    YAddress(const YAddress&);
    
    YAddress():is_null(true){}
    
    explicit operator bool() const
    { return is_null;}

    bool operator==(const YAddress& oth) const
    {
        return  ((this->m_ip==oth.m_ip) && (this->m_port == oth.m_port));
    }

    bool operator!=(const YAddress& oth) const
    {
        return !(*this == oth);
    }
    
    YAddress& operator=(const YAddress& oth);
    YAddress& operator=(YAddress&& oth);


private:
    bool is_null;
};

}

namespace std
{
template<> 
struct hash<yrpc::detail::net::YAddress>
{
    std::size_t operator()(yrpc::detail::net::YAddress const& addr) const
    {
        auto const h1 (std::hash<std::string>{}(addr.GetIP()) );
        auto const h2 (std::hash<int>{}(addr.GetPort()) );
        return h1 ^ (h2 << 1); // 或使用 boost::hash_combine （见讨论）
    }
};
}