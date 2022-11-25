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
        :_ip(ip),
        _port(port)
{
    InitAddress(_addr,port,ip);
}


//服务器用	ip地址配置主要是INADDR_ANY
IPAddress::IPAddress(int port, int opt)
		:_ip(""),
		_port(port)
{
    InitAddress(_addr,port,_ip,opt);
}

//返回string格式的 点分10进制ip地址
std::string IPAddress::GetIP() const
{
    return _ip;
}

//返回一个int格式的port
int IPAddress::GetPort() const
{
    return _port;
}

//设置 ip、port	
void IPAddress::set(std::string ip,int port)
{
    _port = port;
	_ip = ip;

	InitAddress(_addr,port,ip);
}

void IPAddress::set(sockaddr_in addr)
{
    _addr = addr;
	_ip = inet_ntoa(addr.sin_addr);
	_port = ntohs(addr.sin_port);
}


//返回 string {ip:port}
std::string IPAddress::GetIPPort() const 
{

    return _ip+":"+std::to_string(_port);
}

IPAddress::IPAddress(IPAddress &&addr)
	:_addr(addr._addr),
	_ip(std::move(addr._ip)),
	_port(std::move(addr._port))
{
	// addr._addr;	// 懒得初始化了，感觉没必要，承诺用不到就行了
	addr._port = -1;
}

IPAddress::IPAddress(const IPAddress&addr)
	:_addr(addr._addr),
	_ip(addr._ip),
	_port(addr._port)
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
	this->_addr = oth._addr;
	this->_ip = oth._ip;
	this->_port = oth._port;
	this->is_null = this->is_null;
}

YAddress& YAddress::operator=(YAddress&& oth)
{
	this->_addr = oth._addr;
	this->_ip = std::move(oth._ip);
	this->_port = oth._port;
	this->is_null = this->is_null;
	oth.is_null = true;	// 置空表示，移交所有权
}