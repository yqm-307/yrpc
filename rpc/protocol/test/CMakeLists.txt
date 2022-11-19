


set(PROTO_MSG ../../proto/c2s.pb.cc ../../proto/s2c.pb.cc ../../proto/s2c.pb.h ../../proto/c2s.pb.h)

add_executable(test_protocol test_protocol.cc ${RROTO_MSG})
target_link_libraries(test_protocol yrpc ybbt ${PROTOBUF_LIBRARIES})
