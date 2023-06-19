#include "PointServer.h"
#include "yrpc/config/config.hpp"


void routine_handle(void* p2pserv)
{

}

int main(int argc, char* argv[])
{
    if (argc != 4)
    {
        printf("need 3 params, but have %d\n", argc - 1);
        printf("usage: p2pserver [peer ip] [peer port] [listen port]\n");
        exit(-1);
    }
    const int debug = 1;
    const int nthread = 2;
    BBT_CONFIG()->GetDynamicCfg()->SetEntry<int>(bbt::config::BBTSysCfg[bbt::config::BBT_LOG_STDOUT_OPEN], &debug);
    BBT_CONFIG()->GetDynamicCfg()->SetEntry<int>(yrpc::config::SysCfg[yrpc::config::THREAD_NUM], &nthread);

    std::string peer_ip(argv[1]);
    int peer_port = std::stoi(argv[2]);
    int listen_port = std::stoi(argv[3]);

    PointServer p(peer_ip, peer_port, listen_port);
    p.StartService();
    y_scheduler->AddTask([](void* arg){
        auto p2p = static_cast<PointServer*>(arg);
        p2p->Start();
    }, &p);
    y_scheduler->RunForever();
    y_scheduler->Loop();
}