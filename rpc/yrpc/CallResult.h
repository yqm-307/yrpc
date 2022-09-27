// #pragma once
// #include "../msg/servicemap.h"
// #include "../protocol/Codec.h"
// #include "../Util/Locker.h"
// #include <mutex>
// #include <thread>

// namespace yrpc::rpc
// {

// template<class SendType,class RecvType>
// class Result
// {
// public:
//     typedef std::shared_ptr<RecvType> RecvPtr;
//     Result():is_done_(false),
//         call_id_(-1)
//     {}


//     void SetCallId(uint64_t id)
//     { call_id_ = id; }


//     void SetCallObj(std::shared_ptr<CallObj<SendType,RecvType>> obj)
//     {
//         result_ = obj;
//     }

//     int Try_Get(RecvPtr res)
//     {
//         if(CallIsDone())
//         {   
//             res = result_->GetRecv();
//             return 0;
//         }
//         return -1;
//     }

//     /**
//      * @brief 阻塞直到结果返回，可以设置超时时间
//      * 
//      * @param res 结果以智能指针形式返回
//      * @param timeout_ms 超时时间
//      * @return int 返回0说明结果正常，-1表示call尚未注册完毕，-2表示call id 错误，-3表示超时
//      */
//     int Get(RecvPtr res,int timeout_ms = -1)
//     {
//         using namespace yrpc::util::clock;
//         if(timeout_ms<0)
//             timeout_ms = 1;
        
//         if(0 == Try_Get(res))
//             return 0;
//         Timestamp<ms> timeout = nowAfter<ms>(ms(timeout_ms));
//         while(!expired<ms>(timeout))
//         {
//             std::this_thread::sleep_for(ms(5));
//             if(0 == Try_Get(res))
//                 return 0;
//         }
//         return -1;
//     }

// private:
//     bool CallIsDone()
//     {
//         return result_->Is_Done();
//     }
// private:

//     bool is_done_{false};
//     std::shared_ptr<CallObj<SendType,RecvType>> result_;
//     uint64_t call_id_;

// };

// }
