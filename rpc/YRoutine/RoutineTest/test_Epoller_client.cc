#include "../Epoller.h"
#include "../Hook.h"

#include <string.h>
#include <assert.h>
#include <unistd.h>

using namespace yrpc;

class EchoClient
{
public:
    EchoClient(yrpc::coroutine::poller::Epoller& loop,std::string ip,int port)
        :loop_(loop),
        socket_(createSocket(ip,port))
    {
        if(socket_ == nullptr)
            exit(-1);
        loop_.AddTask([this](void*){handle(nullptr);},nullptr);
    }

    yrpc::socket::Socket* createSocket(std::string& ip,int port)
    {
        int sockfd = ::socket(AF_INET,SOCK_STREAM,0);
        
        auto socket = loop_.CreateSocket(sockfd);
        
        
        struct sockaddr_in cliaddr;
        cliaddr.sin_family = AF_INET;
        cliaddr.sin_addr.s_addr = inet_addr(ip.c_str());
        cliaddr.sin_port = htons(port);
        addr_ = cliaddr;
        yrpc::socket::YRSetConnectTimeout(*socket,1000);

        return socket;
    }


    void handle(void*args)
    {
        int ret = yrpc::socket::YRConnect(*socket_,(sockaddr*)&addr_,sizeof(addr_));
        printf("connect : %d\n",ret);
        if(ret != 0)
        {
            yrpc::socket::YRClose(*socket_);
            return;
        }
        
        char recvbuf[1024]{'0'};

        yrpc::socket::YRRead(*socket_,recvbuf,1024);  //登录成功服务器应答
        printf("%s\n",recvbuf);

        for(int i=0;i<10;++i)
        {
            char line[128];
            memset(line,'\0',128);
            printf("\nC>>S:");
            std::cin>>line;
            yrpc::socket::YRSend(*socket_,line,strlen(line),0);
            memset(line,'\0',128);
            yrpc::socket::YRRead(*socket_,line,128); 
            printf("\nS>>C:%s\n",line);
        }

        yrpc::socket::YRSend(*socket_,"quit\n",5,0);
        yrpc::socket::YRClose(*socket_);
        
    }

    void start(){loop_.Loop();}

private:    
    yrpc::coroutine::poller::Epoller& loop_;
    yrpc::socket::Socket* socket_;
    struct sockaddr_in addr_;
};



int main(int argc, char * argv[]) {


    
    yrpc::coroutine::poller::Epoller loop(64*1024,65535);
    EchoClient client(loop,"127.0.0.1",10020);

    client.start();


    return 0;
}

