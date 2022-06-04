
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin/YRoutineTest)


add_executable(YRoutine1  test_YRoutine1.cc)
target_link_libraries(YRoutine1 yrpc boost_context YUtil)
add_executable(YRoutine2  test_YRoutine2.cc)
target_link_libraries(YRoutine2 yrpc boost_context YUtil)
add_executable(RoutineScheduler  test_YCO_Scheduler.cc)
target_link_libraries(RoutineScheduler yrpc boost_context YUtil)