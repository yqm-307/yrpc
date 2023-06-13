// #include "../../proto/test_protocol/AddAndStr.pb.h"
// #include "../YRpc.h"
// #include <memory>
// #include <bbt/config/GlobalConfig.hpp>
// using namespace yrpc;
// using namespace yrpc::util;
// typedef std::function<void(std::shared_ptr<google::protobuf::Message>)> SendPacket;
// typedef std::shared_ptr<google::protobuf::Message> MessagePtr;



// MessagePtr AddHandle(MessagePtr args)
// {
//     std::shared_ptr<AddReq> req = std::static_pointer_cast<AddReq>(args);
//     int rval =  req->a();
//     int lval = req->b();
//     auto rsp = std::make_shared<AddRsp>();
//     rsp->set_result(rval+lval);
//     return rsp;
// }

// MessagePtr EchoHandle(const MessagePtr args)
// {
//     const std::shared_ptr<EchoReq> req = std::static_pointer_cast<EchoReq>(args);
//     std::string str = req->str();
//     auto rsp = std::make_shared<EchoRsp>();
//     rsp->set_str(str);
//     return rsp;
// }

// int main()
// {

//     while(true)
//         std::this_thread::sleep_for(yrpc::util::clock::s(3)); 
// }