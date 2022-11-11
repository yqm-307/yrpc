

set(ProtoFile ../../proto/test_protocol/AddAndStr.pb.cc ../../proto/test_protocol/AddAndStr.pb.h)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -O3 -Wall")

 
add_executable(test_SessionBuffer  test_SessionBuffer.cc ${ProtoFile})
target_link_libraries(test_SessionBuffer yrpc YUtil ${PROTOBUF_LIBRARIES})
