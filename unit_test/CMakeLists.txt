
set(MY_COMM_LIB
bbt_rpc
bbt_core
bbt_network
bbt_pollevent
boost_unit_test_framework
boost_test_exec_monitor
)

add_executable(test1 test1.cc)
target_link_libraries(test1 PRIVATE
    ${MY_COMM_LIB}
)
add_test(NAME test1 COMMAND test1)


add_executable(test2 test2.cc)
target_link_libraries(test2 PRIVATE
    ${MY_COMM_LIB}
)
add_test(NAME test2 COMMAND test2)