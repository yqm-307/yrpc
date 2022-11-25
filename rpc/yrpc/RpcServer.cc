#include "RpcServer.h"

using namespace yrpc::rpc;




class SService
{
public:
    SService(){};
    ~SService(){}
    void Notify()
    { cond.notify_one(); }
    
    void Wait()
    { cond.wait(); }

    std::string& Result()
    { return result; }

    std::string& Request();

private:
    yrpc::util::lock::Sem_t cond;
    std::string request{""};
    std::string result{""};
};
typedef std::shared_ptr<SService> SPtr;










RpcServer::RpcServer()
{
}



RpcServer::~RpcServer()
{
}


void RpcServer::SetAddress(Address&& addr)
{
    m_serv_addr = std::move(addr);
}   

void RpcServer::SetAddress(const Address& addr)
{
    m_serv_addr = addr;
}

void RpcServer::SetThreadPool(int nth)
{
    // 初始化 ServiceMoudle
    yrpc::rpc::detail::Service_Base::GetInstance(true,nth);

}




void RpcServer::Start()
{
    yrpc::rpc::detail::__YRPC_SessionManager::GetInstance()->AsyncAccept(m_serv_addr);
    while(!m_stop.load())
        std::this_thread::sleep_for(yrpc::util::clock::ms(100));        
}