cmake_minimum_required(VERSION 2.6) #最低版本号2.6
project(yqmnet C CXX)               #工程名和工程语言


#自定义变量 环境选项
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -g -w")
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)   #可执行文件目录
set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)      #库输出路径
include_directories(${PROJECT_BINARY_DIR})


add_definitions(-Dcmakebuild)



set(INCLUDE_FILE
rpc/YRoutine/RoutineStack.h
rpc/YRoutine/YRoutineContext_Base.h
rpc/YRoutine/YRoutineContext.h
rpc/YRoutine/YCO_Scheduler.h
)

set(SRC_FILE
rpc/YRoutine/RoutineStack.cc
rpc/YRoutine/YRoutineContext_Base.cc
rpc/YRoutine/YRoutineContext.cc
rpc/YRoutine/YCO_Scheduler.cc
)


add_library(yrpc SHARED ${SRC_FILE} ${INCLUDE_FILE})  #源文件目录
target_link_libraries(yrpc boost_context YUtil pthread )

install(TARGETS yrpc DESTINATION lib)

install(FILES ${INCLUDE_FILE} DESTINATION include)

add_subdirectory(rpc)


