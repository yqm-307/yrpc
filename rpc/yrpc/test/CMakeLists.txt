

set(ProtoFile ../../proto/test_protocol/AddAndStr.pb.cc ../../proto/test_protocol/AddAndStr.pb.h)

 
add_executable(Service_login  Service_login.cc ${ProtoFile})
target_link_libraries(Service_login yrpc ${PROTOBUF_LIBRARIES})