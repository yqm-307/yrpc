
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin/example/cluster)

set(ProtoFile ../all_example.pb.cc ../all_example.pb.h)

add_executable(Center  ServiceCenter.cc ${ProtoFile})
target_link_libraries(Center yrpc ybbt ${PROTOBUF_LIBRARIES})