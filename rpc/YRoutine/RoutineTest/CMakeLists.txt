
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin/YRoutineTest)


add_executable(YRoutine1  test_YRoutine1.cc)
target_link_libraries(YRoutine1 yrpc)

add_executable(YRoutine2  test_YRoutine2.cc)
target_link_libraries(YRoutine2 yrpc)

add_executable(RoutineScheduler  test_YCO_Scheduler.cc)
target_link_libraries(RoutineScheduler yrpc)

add_executable(Epollerclient  test_Epoller_client.cc)
target_link_libraries(Epollerclient yrpc)

add_executable(Epollerserver  test_Epoller_server.cc)
target_link_libraries(Epollerserver yrpc)

add_executable(Cond_t  test_Cond_t.cc)
target_link_libraries(Cond_t yrpc)

add_executable(Sleep  test_Sleep.cc)
target_link_libraries(Sleep yrpc)

add_executable(Suspend  test_Suspend.cc)
target_link_libraries(Suspend yrpc)