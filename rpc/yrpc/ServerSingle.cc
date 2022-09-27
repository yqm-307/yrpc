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
    yrpc::coroutine::poller::Epoller* scheduler_;
    yrpc::socket::Epoll_Cond_t cond;    
    std::string req_bytes_;
    std::string sdata_{""};
};
typedef std::shared_ptr<SCallObj> CallPtr;


ServerSingle::ServerSingle(yrpc::coroutine::poller::Epoller* scheduler,int port,
                            int socket_timeout_ms,int connect_timeout_ms,int stack_size)
    :connect_timeout_ms_(connect_timeout_ms),
    socket_timeout_ms_(socket_timeout_ms),
    scheduler_(scheduler),
    acceptor_(scheduler_,port,socket_timeout_ms_,connect_timeout_ms_),
    closed_(false)
{
    assert(scheduler_!=nullptr);
    //注册onconnect回调
    //启动监听套接字
    acceptor_.setOnConnect(std::bind(&ServerSingle::OnConnHandle,this,yrpc::detail::ynet::_1,yrpc::detail::ynet::_2));
    
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
    closed_ == true;
}

void ServerSingle::OnSendHandle(const yrpc::detail::ynet::ConnectionPtr&conn,void*args)
{
 
}


void ServerSingle::OnConnHandle(const yrpc::detail::ynet::ConnectionPtr&conn,void*)
{
    using namespace yrpc::util::clock;
    //协程局部变量

    yrpc::util::buffer::Buffer bytes_;
    std::mutex lock;
    yrpc::socket::Epoll_Cond_t cond;
    std::atomic_bool task_is_done_{false};
    char *buf = (char *)malloc(4096 * 2); //一条连接独占4kb缓冲
    yrpc::detail::ynet::SessionBuffer package;

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
        memset(buf,'\0',sizeof(buf));
        static int a=0;
        int ret = conn->recv(buf,4096);
        package.Save(buf,ret);

        if(ret <= 0)    //读取错误，等待下次循环检查isclosed
            continue;   


        while(true)
        {
            std::string data = package.GetAReq();

            if (data.size() != 0)
            {
                CallPtr call = std::make_shared<SCallObj>(scheduler_);   //创建callobj
                call->Set_Req_Bytes(data);

                std::string tmp = caller_.Service(data);
                int n = conn->send(tmp.data(),tmp.size());
                // DEBUG("发送L %ld\n",n);

                // scheduler_->AddTask([call, &conn](void *args)
                // {
                //     auto ptr = (yrpc::util::buffer::Buffer*)args;
                //     call->Wait();   //等待 Service 结束
                //     static uint64_t nbyte=0;
                //     int n = conn->send(call->GetBytes().c_str(),call->GetBytes().size());
                //     //DEBUG("总发送: %ld\n",nbyte+=n);
                // },&bytes_);


                // t_pool_.AddTask([this, call]()
                // {
                //     call->GetBytes() = caller_.Service(call->GetReq());
                //     call->Notify(); 
                // });
            }
            else
                break;
        }
        
    }
    free(buf);
}
