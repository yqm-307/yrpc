

set(ProtoFile ../../proto/AddAndStr.pb.cc ../../proto/AddAndStr.pb.h)
 
 
add_executable(RpcServer_test  Server.cc ${ProtoFile})
target_link_libraries(RpcServer_test yrpc YUtil ${PROTOBUF_LIBRARIES})

add_executable(RpcClient_test  Client.cc ${ProtoFile})
target_link_libraries(RpcClient_test yrpc YUtil ${PROTOBUF_LIBRARIES})