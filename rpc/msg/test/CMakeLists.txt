
 
# Find required protobuf package
find_package(Protobuf REQUIRED)
if(PROTOBUF_FOUND)
    message(STATUS "protobuf library found")
else()
    message(FATAL_ERROR "protobuf library is needed but cant be found")
endif()
 
PROTOBUF_GENERATE_CPP(PROTO_SRCS PROTO_HDRS addr.proto)
 
add_executable(Servicetest  Service_test.cc ${PROTO_SRCS} ${PROTO_HDRS})
target_link_libraries(Servicetest yrpc ${PROTOBUF_LIBRARIES})
