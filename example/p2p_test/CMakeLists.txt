include_directories(${PROJECT_SOURCE_DIR})


set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin/example/cluster)

set(ProtoFile ../all_example.pb.cc ../all_example.pb.h)

add_executable(p2pserver  PointServer.cc ${ProtoFile})
target_link_libraries(p2pserver yrpc ybbt ${PROTOBUF_LIBRARIES})