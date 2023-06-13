
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/example/util_test)


add_executable(util_test_buffer test_buffer.cc)
target_link_libraries(util_test_buffer yrpc ybbt)


add_executable(util_test_ID test_ID.cc)
target_link_libraries(util_test_ID yrpc ybbt)


add_executable(util_test_logger test_logger.cc)
target_link_libraries(util_test_logger yrpc ybbt)


add_executable(util_test_thread_safe test_thread_safe.cc)
target_link_libraries(util_test_thread_safe yrpc ybbt)


add_executable(util_test_timeutil test_timeutil.cc)
target_link_libraries(util_test_timeutil yrpc ybbt)


add_executable(util_test_type test_type.cc)
target_link_libraries(util_test_type yrpc ybbt)


add_executable(util_test_YAssert test_YAssert.cc)
target_link_libraries(util_test_YAssert yrpc ybbt)


add_executable(util_YTimertest YTimertest.cc)
target_link_libraries(util_YTimertest yrpc ybbt)