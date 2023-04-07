cmake_minimum_required(VERSION 3.2) # 这里的话最低版本可以是2.8
project(libyrpc)

# SET(CMAKE_BUILD_TYPE "Release")
# 工程名和工程语言
enable_language(C CXX)

# 自定义变量 环境选项
set(Wall_Flag "-Wall -Wno-sign-compare -Wno-format -Wno-reorder")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-rtti -std=c++17 -g ${Wall_Flag}")

# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-rtti -std=c++17 -g -Wall")
# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -fno-rtti -std=c++17 -g -Wall")
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin) # 可执行文件目录
set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib) # 库输出路径

# include 包含路径
include_directories(
    # # 下面两个都 include ，为了兼容 vscode cmake tools
    ${PROJECT_SOURCE_DIR}/rpc/proto # 在主目录构建
    ${PROJECT_SOURCE_DIR}/third-lib/boost_contex/include
)

# lib 文件 包含路径
link_directories(
    ${PROJECT_BINARY_DIR}/../third-lib/lib
    ${PROJECT_SOURCE_DIR}/third-lib/lib
)
add_definitions(-Dcmakebuild)
# 设置输出路径
set(MESSAGE_DIR ${CMAKE_BINARY_DIR}/rpc/proto)

# 是否需要创建目录
if(EXISTS "${CMAKE_BINARY_DIR}/rpc/proto" AND IS_DIRECTORY "${CMAKE_BINARY_DIR}/rpc/proto")
    SET(DST_DIR ${MESSAGE_DIR})
else()
    file(MAKE_DIRECTORY ${MESSAGE_DIR})
    SET(DST_DIR ${MESSAGE_DIR})
endif()

# 设置protoc的搜索路径
LIST(APPEND PROTO_FLAGS -I${CMAKE_CURRENT_SOURCE_DIR})

# 获取需要编译的proto文件
file(GLOB MSG_PROTOS ${CMAKE_BINARY_DIR}/message/*.proto) # 搜索所有proto文件

file(GLOB SRC_FILE
    rpc/YRoutine/*.cc

    # ${PROJECT_BINARY_DIR}/rpc/YRoutine/*.S  # 暂时没啥用了，安心用boost context
    rpc/Util/*.cc
    rpc/msg/*.cc
    rpc/network/*.cc
    rpc/protocol/*.cc
    rpc/proto/*/*.cc # protobuf 生成头文件
    rpc/yrpc/*.cc
)

# 生成 yrpc 的 lib文件
add_library(yrpc SHARED ${SRC_FILE})

# protobuf lib 依赖
find_package(Protobuf REQUIRED)

if(PROTOBUF_FOUND)
    message(STATUS "protobuf library found")
else()
    message(FATAL_ERROR "protobuf library is needed but cant be found")
endif()

# yrpc 的依赖库
target_link_libraries(yrpc 
    ${PROTOBUF_LIBRARIES} 
    boost_context 
    pthread
)

install(TARGETS yrpc DESTINATION lib)

install(FILES ${INCLUDE_FILE} DESTINATION include)

add_subdirectory(rpc)