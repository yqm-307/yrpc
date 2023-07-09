#include "IPAddress.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>

using namespace yrpc::detail::net;


void InitAddress(sockaddr_in& addr ,int port,std::string ip="",int opt = INADDR_ANY)
{
	bzero(&addr,sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	if(ip == "")
	{
		addr.sin_addr.s_addr = htonl(opt);
	}
	else
	{
		inet_aton(ip.c_str(),&addr.sin_addr); //ip
	}
	
}

//客户端用
IPAddress::IPAddress(std::string ip,int port)
        :m_ip(ip),
        m_port(port)
{
    InitAddress(m_addr,port,ip);
}


//服务器用	ip地址配置主要是INADDR_ANY
IPAddress::IPAddress(int port, int opt)
		:m_ip(""),
		m_port(port)
{
    InitAddress(m_addr,port,m_ip,opt);
}

//返回string格式的 点分10进制ip地址
std::string IPAddress::GetIP() const
{
    return m_ip;
}

//返回一个int格式的port
int IPAddress::GetPort() const
{
    return m_port;
}

//设置 ip、port	
void IPAddress::set(std::string ip,int port)
{
    m_port = port;
	m_ip = ip;

	InitAddress(m_addr,port,ip);
}

void IPAddress::set(sockaddr_in addr)
{
    m_addr = addr;
	m_ip = inet_ntoa(addr.sin_addr);
	m_port = ntohs(addr.sin_port);
}


//返回 string {ip:port}
std::string IPAddress::GetIPPort() const 
{

    return m_ip+":"+std::to_string(m_port);
}

IPAddress::IPAddress(IPAddress &&addr)
	:m_addr(addr.m_addr),
	m_ip(std::move(addr.m_ip)),
	m_port(std::move(addr.m_port))
{
	// addr._addr;	// 懒得初始化了，感觉没必要，承诺用不到就行了
	addr.m_port = -1;
}

IPAddress::IPAddress(const IPAddress&addr)
	:m_addr(addr.m_addr),
	m_ip(addr.m_ip),
	m_port(addr.m_port)
{
}



YAddress::YAddress(YAddress&&addr)
	:IPAddress(std::move(addr)),
	is_null(addr.is_null)
{
	addr.is_null = true;
}

YAddress::YAddress(const YAddress&addr)
	:IPAddress(addr),
	is_null(addr.is_null)
{
}


YAddress& YAddress::operator=(const YAddress& oth)
{
	this->m_addr = oth.m_addr;
	this->m_ip = oth.m_ip;
	this->m_port = oth.m_port;
	this->is_null = this->is_null;
	return *this;
}

YAddress& YAddress::operator=(YAddress&& oth)
{
	this->m_addr = oth.m_addr;
	this->m_ip = std::move(oth.m_ip);
	this->m_port = oth.m_port;
	this->is_null = this->is_null;
	oth.is_null = true;	// 置空表示，移交所有权
	return *this;
}