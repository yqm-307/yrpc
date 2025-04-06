

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/build/bin/benchmark/fatiguetest)

add_executable(echo_client fatiguetest/echo_client.cc)
target_link_libraries(echo_client
    yrpc
    bbt_core
    bbt_pollevent
    bbt_network
)

add_executable(echo_server fatiguetest/echo_server.cc)
target_link_libraries(echo_server
    yrpc
    bbt_core
    bbt_pollevent
    bbt_network
)