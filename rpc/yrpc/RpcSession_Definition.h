// #include "../protocol/ProtocolUtil.h"


// template <class SendType, class RecvType>
// RpcSession<SendType, RecvType>::RpcSession(yrpc::coroutine::poller::Epoller *sche, std::string name, yrpc::detail::ynet::YAddress addr, int timeout)
//     : scheduler_(sche),
//       connector_(sche, addr),
//       name_(name),
//       connected_(false)
// {
//     int ret = cond_t_.Init(scheduler_, timeout);
//     if (ret < 0)
//         FATAL("Future::Future() error , sys_call pipe() fatal!");

//     //建立连接
//     connector_.setOnConnect([this](const yrpc::detail::ynet::ConnectionPtr &conn, void *sendpkg)
//                             { OnConnHandle(conn); });
//     connector_.setClosed([this]()
//                          { this->cond_t_.Notify(); });
//     connector_.connect();
// }

// template <class SendType, class RecvType>
// typename RpcSession<SendType, RecvType>::CallPtr RpcSession<SendType, RecvType>::Call(SendPtr sendpkg)
// {
//     if (callmap_.size() >= max_size_ - 1)
//         return nullptr;
//     uint64_t id = yrpc::util::id::GenerateID::GetIDuint64();
//     CallPtr obj = std::make_shared<CallObj<SendType, RecvType>>(scheduler_, sendpkg);

//     std::string bytes{""};
//     resultmap_.insert(CallSlot(id,obj));        //加入结果池
//     bool success = Encode(bytes,id,(&*obj->GetSend()));
//     if(success){
//         output_.WriteString(bytes.c_str(),bytes.size());
//     }
//     else
//         ERROR("RpcSession::Call() error , request Encode error!");
//     return obj;
// }

// template <class SendType, class RecvType>
// uint64_t RpcSession<SendType, RecvType>::Async_Call(SendPtr ptr, std::function<void(RecvType &)> f)
// {
//     assert(f != nullptr);
//     if (callmap_.size() >= max_size_ - 1)
//         return 0;
//     uint64_t id = yrpc::util::id::GenerateID::GetIDuint64();
//     CallPtr obj = std::make_shared<CallObj<SendType, RecvType>>(scheduler_, ptr);
//     obj->SetFunc(f);    //设置CallObj为异步调用模式

//     std::string bytes{""};
//     resultmap_.insert(CallSlot(id,obj));
//     DEBUG("注册异步日志!");
//     bool success = Encode(bytes,id,(&*obj->GetSend()));
//     if(success){
//         output_.WriteString(bytes.c_str(),bytes.size());
//     }
//     else
//         ERROR("RpcSession::Call() error , request Encode error!");
    
//     return id;
// }


// template <class SendType, class RecvType>
// bool RpcSession<SendType, RecvType>::Encode(std::string &bytes, uint64_t call_id, const google::protobuf::Message *msg)
// {
//     /*
//      *   报文结构
//      *   | int32(length) | uint32_t(服务id) | uint64_t(消息id) |protobuf(数据正文) |
//      */
//     uint32_t service_id = yrpc::util::hash::BKDRHash(name_.c_str(), name_.size());
//     bytes.resize(sizeof(int)*4,'0'); //预留消息头

//     bool ok = msg->AppendToString(&bytes);
//     yrpc::detail::YRPCProtocol::GenerateMsg(yrpc::detail::YRPC_PROTOCOL_MESSAGE,service_id,call_id,bytes);  //生成报文

//     return ok;
// }

// template <class SendType, class RecvType>
// std::string RpcSession<SendType, RecvType>::DeCode(const char *data, size_t len)
// {
//     package_.Save(data, len);
//     std::string ret{""};
//     ret = package_.GetAReq();

//     return ret;
// }

// template <class SendType, class RecvType>
// void RpcSession<SendType, RecvType>::SetResult(CALLTYPE code, CallPtr ptr, uint64_t id, RecvPtr result)
// {

//     assert(id >= 0);
//     if (code == CALL_IS_END && result != nullptr) //调用成功
//     {
//         ptr->SetRecv(result); //保存结果
//         //是否为异步调用
//         if (ptr->is_async_call())
//         {
//             ptr->Async_Call();
//         }
//         else
//         { //future调用
//             ptr->SetType(code);   //改变状态
//         }
//     }
//     else
//     {
//         //成功or失败，都要从调用池删除，本次调用已经结束
//         ptr->SetRecv(nullptr);
//         ptr->Async_Call();
//         ptr->SetType(CALL_IS_FATAL);
//     }


//     resultmap_.erase(id);
//     return;
// }

// template <class SendType, class RecvType>
// void RpcSession<SendType, RecvType>::SetErrno(CALLTYPE code,CallPtr ptr,uint64_t id,int errcode)
// {
//     ptr->SetErrCode(errcode);
//     ptr->SetType(code);
//     auto it = resultmap_.erase(id);
// }


// template <class SendType, class RecvType>
// void RpcSession<SendType, RecvType>::OnConnHandle(const yrpc::detail::ynet::ConnectionPtr &conn)
// {
//     using namespace yrpc::util::clock;
//     connected_ = true;
    
//     //开启Input协程
//     scheduler_->AddTask([this, &conn](void *)
//                         { this->OnRecvHandle(conn); },
//                         nullptr);
    

//     Timestamp<ms> timeslice = nowAfter<ms,Timestamp<ms>>(ms(1));    //分配1ms运行时间片 

//     while (!conn->IsClosed())
//     {
//         //当前队列没有请求，挂起
//         while (output_.ReadableBytes() == 0){
//             yrpc::socket::YRSleep(scheduler_,2);
//             timeslice = nowAfter<ms,Timestamp<ms>>(ms(1));
//         }

//         if(expired<ms,Timestamp<ms>>(timeslice))
//         {//时间片消耗完
//             scheduler_->Yield();
//             timeslice = nowAfter<ms,Timestamp<ms>>(ms(1));
//         }

//         memset(buf, '\0', sizeof(buf));

//         while(output_.ReadableBytes() >= 0)
//         {
//             conn->send(output_.peek(),1440);
//             output_.recycle(1440);
//         }

//     }
//     connected_ = false;
// }


// template <class SendType, class RecvType>
// void SendMsgHandle()
// {

// }


// template <class SendType, class RecvType>
// void RpcSession<SendType, RecvType>::OnRecvHandle(const yrpc::detail::ynet::ConnectionPtr &conn)
// {
//     using namespace yrpc::util::clock;

//     scheduler_->AddTask([this](void *){this->MsgParseHandle();},nullptr);   //注册解析任务
//     /*
//     *   将数据接收并保存到接受缓存
//     */
//     //最长执行1ms
//     Timestamp<ms> timeslice = nowAfter<ms,Timestamp<ms>>(ms(1));    //分配1ms运行时间片 

//     while (!conn->IsClosed())
//     {
//         if(expired<ms,Timestamp<ms>>(timeslice))
//         {//时间片消耗完
//             scheduler_->Yield();
//             timeslice = nowAfter<ms,Timestamp<ms>>(ms(1));
//         }
//         int n = 0;
//         static uint64_t nbytes=0;
//         memset(recvbuf, '\0', sizeof(recvbuf));
//         n = conn->recv(recvbuf, sizeof(recvbuf));
//         //("总接受 : %ld\n",nbytes+=n);
//         package_.Save(recvbuf,n);
//     }
// }

// template <class SendType, class RecvType>
// void RpcSession<SendType, RecvType>::MsgParseHandle()
// {
//     using namespace yrpc::util::clock;

//     Timestamp<ms> timeslice = nowAfter<ms,Timestamp<ms>>(ms(1));    //分配1ms运行时间片 
//     while(connected_)
//     {//连接建立期间
        
//         if(expired<ms,Timestamp<ms>>(timeslice))
//         {//时间片消耗完
//             scheduler_->Yield();
//             timeslice = nowAfter<ms,Timestamp<ms>>(ms(1));
//         }
//         //不断解析pkg做错误处理
//         if(!package_.Empty())
//         {
//             int time = millisecond();
//             /*取出一个package*/
//             std::string bytes =package_.GetAReq();
//             if(bytes.size() <= 0)
//             {
//                 //ERROR("RpcSession::MsgParseHandle() error ,  Package::GetAReq() fatal!");
//                 continue;
//             }

//             uint64_t uid = yrpc::util::protoutil::BytesToType<uint64_t>(bytes.data()+sizeof(int)*2);

//             auto it = resultmap_.find(uid);
//             if(it == resultmap_.end())
//             {
//                 ERROR("RpcSession::MsgParseHandle() error ,  uid parse error!");
//             }

//             std::string_view msg(bytes);
//             int errcode=0;
//             auto recvptr = yrpc::detail::YRPCProtocol::Protocol_Parse<RecvType>(msg,msg.length(),&errcode);
//             if(recvptr == nullptr)
//             {
//                 //错误信息
//                 if(errcode < -2)
//                 {
//                     ERROR("RpcSession::MsgParseHandle() error , server parse error!");
//                 }
//                 SetErrno(CALL_IS_FATAL,it->second,it->first,errcode);//错误信息通知

//             }
//             else
//             {
//                 SetResult(CALL_IS_END,it->second,uid,recvptr);
//             }
//             //DEBUG("解析耗时 : %d ms",millisecond()-time);
//             /*结果(或错误结果)回调通知*/
//         }
//         else
//         {
//             yrpc::socket::YRSleep(scheduler_,0);
//         }
//     }   
// }



// template<class SendType,class RecvType>
// int RpcSession<SendType,RecvType>::SendProtocol(SendPtr ptr)
// {
    
// }
