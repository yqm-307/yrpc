

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/build/bin/example)

add_executable(client client.cc)
target_link_libraries(client
    bbt_rpc
    bbt_core
    bbt_pollevent
    bbt_network
)

add_executable(server server.cc)
target_link_libraries(server
    bbt_rpc
    bbt_core
    bbt_pollevent
    bbt_network
)