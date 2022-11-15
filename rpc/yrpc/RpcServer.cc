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










RpcServer::RpcServer(int port,size_t threadnum,std::string logpath,int socket_timeout_ms,int connect_timeout_ms,int stack_size,int maxqueue)
    :m_serv_addr(port)
{



}



RpcServer::~RpcServer()
{
}


void RpcServer::SetThreadPool(ThreadPool* pool)
{
    m_pool = pool;
}



void RpcServer::Dispatch(std::string& bytearray,detail::RpcSession::SessionPtr sess)
{
    if(sess->IsClosed())
        return;
    
    /**
     * 如果没有线程池，就是同步在IO线程中执行
     * 如果有线程池，就是注册在线程池冲的异步任务，完成后自动发送(其实线程池中是又注册一个IO任务到IO线程)
     */
    if (m_pool != nullptr)
    {   
        m_pool->AddTask([bytearray,sess](){
            auto&& result = detail::CallCenter::Service(bytearray);
            if(!sess->IsClosed())
                sess->Append(result);
        });
    }
    else
    {
        auto &&result = detail::CallCenter::Service(bytearray);
        if (!sess->IsClosed())
            sess->Append(result);
    }
}


void RpcServer::Start()
{
    yrpc::rpc::detail::__YRPC_SessionManager::GetInstance()->AsyncAccept(m_serv_addr,[this](std::string& bytearray,detail::RpcSession::SessionPtr sess)
        {
            this->Dispatch(bytearray,sess);
        }
    );
    while(!m_stop.load())
        std::this_thread::sleep_for(yrpc::util::clock::ms(100));        
}