
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin/YRoutineTest)


add_executable(YRoutine  test_YRoutine.cc)
target_link_libraries(YRoutine yrpc boost_context YUtil)