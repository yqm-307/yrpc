#include "RpcServer.h"

using namespace yrpc::rpc;

RpcServer::RpcServer(int port,size_t threadnum,std::string logpath,int socket_timeout_ms,int connect_timeout_ms,int stack_size,int maxqueue)
    :port_(port),
    socket_timeout_ms_(socket_timeout_ms),
    connect_timeout_ms_(connect_timeout_ms),
    thread_num_(threadnum),
    scheduler_(new yrpc::coroutine::poller::Epoller(64*1024,65535,logpath)),
    MainServer_(new detail::ServerSingle(scheduler_,port,connect_timeout_ms_,socket_timeout_ms_,nullptr,stack_size)),
    SubServers_(threadnum),
    Threads_(threadnum)
{
    assert(scheduler_);

    if(thread_num_>1)
        for(int i =1 ;i<thread_num_;++i)
            CreateSubServerThread(stack_size,maxqueue);

}



RpcServer::~RpcServer()
{
    delete scheduler_;
    for(int i=0;i<sches_.size();++i)
    {
        delete sches_[i];
    }
}




void RpcServer::start()
{
    scheduler_->RunForever();
    scheduler_->Loop();
}


void RpcServer::CreateSubServerThread(int stack, int queuesize)
{

    for(int i=0;i<thread_num_;++i)
    {
        yrpc::coroutine::poller::Epoller* scheduler_ = new yrpc::coroutine::poller::Epoller(stack,queuesize);
        sches_.push_back(scheduler_);
        SubServers_[i] = new detail::ServerSingle(scheduler_,port_,socket_timeout_ms_,connect_timeout_ms_,t_pool_,stack);
    }

    for(int i=0;i<thread_num_;++i)
    {
        Threads_[i] = new std::thread([&](){
            SubServers_[i]->run();
        });
    }

}


int RpcServer::SetThreadPool(yrpc::util::threadpool::ThreadPool<WorkFunc>* pool)
{
    if(t_pool_ != nullptr)
        return -1;
    t_pool_ = pool;
    return 0;    
}



const yrpc::util::threadpool::ThreadPool<RpcServer::WorkFunc>* RpcServer::GetThreadPool()
{
    return t_pool_;
}

