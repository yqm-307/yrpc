#include "PointServer.h"


void routine_handle(void* p2pserv)
{

}

int main(int argc, char* argv[])
{
    if (argc != 3)
    {
        printf("usage: p2pserver [peer ip] [peer port] [listen port]");
        exit(-1);
    }

    std::string peer_ip(argv[1]);
    int peer_port = std::stoi(argv[2]);
    int listen_port = std::stoi(argv[3]);

    PointServer p(peer_ip, peer_port, listen_port);
    p.StartService();
    _co_scheduler->AddTask([](void* arg){
        auto p2p = static_cast<PointServer*>(arg);
        p2p->Start();
    }, &p);
    _co_scheduler->RunForever();
    _co_scheduler->Loop();
}