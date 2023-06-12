#include "../Epoller.h"
#include "../Hook.h"

#include <string.h>
#include <assert.h>
#include <unistd.h>


class EchoServer
{
public:
    EchoServer(int port,yrpc::coroutine::poller::Epoller& loop)
        :loop_(loop)
    {
        loop_.AddTask([this](void*){
            OnAccept();
        });
        int lsocket;
        yrpc::socket::YRCreateListen(&lsocket,port);
        listen_ = loop_.CreateSocket(lsocket);


    }
    ~EchoServer()
    {
        loop_.DestorySocket(listen_);
    }

    void run()
    {
        loop_.RunForever();
        loop_.Loop();
    }


private:

    yrpc::socket::Socket* createSocket(std::string&& ip,int port)
    {
        int sockfd = ::socket(AF_INET,SOCK_STREAM,0);
        
        auto socket = loop_.CreateSocket(sockfd);
        
        
        struct sockaddr_in cliaddr;
        cliaddr.sin_family = AF_INET;
        cliaddr.sin_addr.s_addr = INADDR_ANY;
        cliaddr.sin_port = htons(port);
        addr_ = cliaddr;
        yrpc::socket::YRSetConnectTimeout(*socket,1000);

        return socket;
    }

    void OnAccept()
    {
        socklen_t len = sizeof(addr_);
        int sockfd = yrpc::socket::YRAccept(*listen_,(sockaddr*)&addr_,&len);
        auto newsocket = loop_.CreateSocket(sockfd);
        if(sockfd < 0)
        {
            perror("socket fd < 0\n");
            exit(-1);
        }

        while(1)
        {
            char buff[4096];
            int n = yrpc::socket::YRRead(*newsocket,buff,sizeof(buff));
            if( n <= 0)
            {
                printf("read ret %d \n",n);
                exit(-2);
            }
            else{
                yrpc::socket::YRWrite(*newsocket,buff,n);
            }
        }
    }



private:
    yrpc::coroutine::poller::Epoller& loop_;
    // yrpc::socket::Socket* socket_;
    yrpc::socket::Socket* listen_;
    struct sockaddr_in addr_;
};


int main()
{
    
    yrpc::coroutine::poller::Epoller loop(64*1024,65535);
    EchoServer server(10020,loop);

    server.run();
}