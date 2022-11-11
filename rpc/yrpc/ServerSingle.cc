#include "ServerSingle.h"
#include "../network/SessionBuffer.h"

using namespace yrpc::rpc::detail;




class SCallObj
{
public:
    SCallObj(yrpc::coroutine::poller::Epoller* sche)
        :scheduler_(sche)
    {
        cond.Init(sche);
    };
    ~SCallObj(){}
    void Notify()
    { cond.Notify(); }
    
    void Wait()
    { cond.Wait(); }

    std::string& GetBytes()
    { return sdata_; }

    void Set_Req_Bytes(std::string_view bytes)
    { req_bytes_ = bytes; }

    std::string& GetReq()
    { return req_bytes_; }
private:
    [[maybe_unused]] yrpc::coroutine::poller::Epoller* scheduler_;
    yrpc::socket::Epoll_Cond_t cond;    
    std::string req_bytes_;
    std::string sdata_{""};
};
typedef std::shared_ptr<SCallObj> CallPtr;


ServerSingle::ServerSingle(yrpc::coroutine::poller::Epoller* scheduler,int port,
                            int socket_timeout_ms,int connect_timeout_ms,yrpc::util::threadpool::ThreadPool<WorkFunc>* threadpool,int stack_size)
    :socket_timeout_ms_(socket_timeout_ms),
    connect_timeout_ms_(connect_timeout_ms),
    scheduler_(scheduler),
    t_pool_(threadpool),
    acceptor_(scheduler_,port,socket_timeout_ms_,connect_timeout_ms_),
    closed_(false)
{
    assert(scheduler_!=nullptr);
    //注册onconnect回调
    //启动监听套接字
    acceptor_.setOnConnect([this](const yrpc::detail::shared::errorcode&e,yrpc::detail::net::ConnectionPtr conn){
        this->OnConnHandle(conn,nullptr);
    });
    
}


void ServerSingle::run()
{
    while(!closed_)
    {
        scheduler_->Loop(); //运行一次
    }
}

void ServerSingle::close()
{
    closed_ = true;
}

void ServerSingle::OnSendHandle(const yrpc::detail::net::ConnectionPtr&conn,void*args)
{
 
}


void ServerSingle::OnConnHandle(const yrpc::detail::net::ConnectionPtr&conn,void*)
{
    using namespace yrpc::util::clock;
    //协程局部变量

    yrpc::util::buffer::Buffer bytes_;
    [[maybe_unused]] std::mutex lock;
    yrpc::socket::Epoll_Cond_t cond;
    [[maybe_unused]] std::atomic_bool task_is_done_{false};
    const int buffersize = 4096;
    char *buf = (char *)malloc(buffersize * 2); //一条连接独占4kb缓冲
    yrpc::detail::net::SessionBuffer package;

    // scheduler_->AddTask([this,conn](void*args){
    //     this->OnSendHandle(conn,args);
    // },&bytes_);


    Timestamp<ms> timeslice = nowAfter<ms,Timestamp<ms>>(ms(1));    //分配1ms运行时间片 

    //conn尚未关闭之前
    while(!conn->IsClosed())
    {   
        if(expired<ms,Timestamp<ms>>(timeslice))
        {//时间片消耗完
            scheduler_->Yield();
            timeslice = nowAfter<ms,Timestamp<ms>>(ms(1));
        }



        //接受原始数据并加入到pkg管理中        
        memset(buf,'\0',buffersize);
        [[maybe_unused]] static int a=0;
        int ret = conn->recv(buf,buffersize);
        package.Append(buf,ret);

        if(ret <= 0)    //读取错误，等待下次循环检查isclosed
            continue;   


        while(true)
        {
            std::string data = package.GetAPck();

            if (data.size() != 0)
            {
                CallPtr call = std::make_shared<SCallObj>(scheduler_);   //创建callobj
                call->Set_Req_Bytes(data);

                if(t_pool_ != nullptr)
                {
                    scheduler_->AddTask([call, &conn](void *args)
                                        {
                        // auto ptr = (yrpc::util::buffer::Buffer*)args;
                        call->Wait();   //等待 Service 结束
                        // static uint64_t nbyte=0;
                        [[maybe_unused]] int n = conn->send(call->GetBytes().c_str(),call->GetBytes().size());
                        //DEBUG("总发送: %ld\n",nbyte+=n); 
                    },&bytes_);

                    t_pool_->AddTask([this, call]()
                    {
                        call->GetBytes() = caller_.Service(call->GetReq());
                        call->Notify(); 
                    });
                }
                else
                {
                    std::string tmp = caller_.Service(data);
                    [[maybe_unused]] int n = conn->send(tmp.data(), tmp.size());
                    // DEBUG("发送L %ld\n",n);
                }
            }
            else
                break;
        }
        
    }
    free(buf);
}
