

find_package(Protobuf REQUIRED)
if(PROTOBUF_FOUND)
    message(STATUS "protobuf library found")
else()
    message(FATAL_ERROR "protobuf library is needed but cant be found")
endif()
 
PROTOBUF_GENERATE_CPP(PROTO_SRCS PROTO_HDRS testproto.proto)
 
add_executable(Service_login  Service_login.cc ${PROTO_SRCS} ${PROTO_HDRS})
target_link_libraries(Service_login yrpc ${PROTOBUF_LIBRARIES})