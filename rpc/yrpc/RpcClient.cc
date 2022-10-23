#include "./RpcClient.h"
#include <memory>

using namespace yrpc::rpc;


RpcClient::RpcClient(std::string ip,int port,std::string logpath,int stack_size,int maxqueue)
    :servaddr_(ip,port),
    close_(false),
    // scheduler_(new yrpc::coroutine::poller::Epoller(stack_size,maxqueue,logpath)),
    // connector_(scheduler_,servaddr_),
    session_(nullptr)
{   

    // 检查连接是否存在于 sessionManager 中，如果存在，直接将 RpcSession 添加到

    // assert(scheduler_!=nullptr);
    thread_ = new std::thread(RpcClient::run,this);
    assert(thread_);
    // connector_.setOnConnect([this](yrpc::detail::net::ConnectionPtr conn,void*){NewConnection(conn);});
}

RpcClient::~RpcClient()
{
    /*
        直接delete IO线程，不安全，应该保证当前尚未完成的调用失败或者等待返回(优雅关闭或者强制关闭)
        解决方法：close 接口，不显式调用析构断开连接。
    */
    if(!isclose())
    {
        close();
        thread_->join();
    }
    delete thread_;
}


void RpcClient::run(void* th)
{
    // RpcClient* tt = (RpcClient*)th;
    // tt->connector_.connect();
    // while(!tt->close_)
    //     tt->scheduler_->Loop();

}

void RpcClient::close()
{
    close_.exchange(true);
}


bool RpcClient::isclose()
{
    return close_.load();
}

void RpcClient::NewConnection(yrpc::detail::net::ConnectionPtr new_conn)
{
    if (!new_conn->IsClosed())
    {
        // session_ = std::make_shared<yrpc::rpc::detail::RpcClientSession>(scheduler_,new_conn);
    }
    else{
        ERROR("RpcClient::NewConnection() error , is closed!");
    }
}


bool RpcClient::async_call(std::string name,std::shared_ptr<google::protobuf::Message> send,yrpc::rpc::detail::RpcCallback f)
{

    // SessionManager::GetInstance()->SessionIsAlive();

    // if(!session_ || !session_->IsConnected())
    // {
    //     ERROR("RpcClient::async_call error , info: clinet session bad!");
    //     return false; 
    // }
    // if(session_->RpcAsyncCall(send,std::move(f)))
    //     return true;
    // else
    //     return false;
}


void RpcClient::OnConnect(const errcode& e,RpcSession* sess)
{
    if (sess != nullptr)
        m_session = sess;

    if(e.err() != yrpc::detail::shared::ERR_NETWORK_CONN_OK)
    {
        assert(sess != nullptr);
        m_session = sess;
    }
    else
    {
        INFO("%s:%s , info:%s",__FUNCTION__,__LINE__,e.what().c_str());
    } 
}
