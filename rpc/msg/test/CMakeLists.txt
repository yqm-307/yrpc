


set(PROTO_SRCS "../../proto/test_protocol/addr.pb.cc")
 
add_executable(Servicetest  Service_test.cc ${PROTO_SRCS} ${PROTO_HDRS})
target_link_libraries(Servicetest yrpc ${PROTOBUF_LIBRARIES})
