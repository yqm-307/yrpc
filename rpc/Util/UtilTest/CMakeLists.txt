


add_executable(YTimer  YTimertest.cc)
target_link_libraries(YTimer yrpc boost_context ybbt)

add_executable(test_timeutil  test_timeutil.cc)
target_link_libraries(test_timeutil yrpc boost_context ybbt)

add_executable(test_log  test_logger.cc)
target_link_libraries(test_log yrpc boost_context ybbt)

add_executable(test_ID  test_ID.cc)
target_link_libraries(test_ID yrpc boost_context ybbt)

add_executable(test_type test_type.cc)
target_link_libraries(test_type yrpc)

add_executable(test_cter test_thread_safe.cc)
target_link_libraries(test_cter yrpc ybbt)
