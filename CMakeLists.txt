cmake_minimum_required(VERSION 3.2) #最低版本号2.6
project(libyrpc)           
#SET(CMAKE_BUILD_TYPE "Release")
#工程名和工程语言
enable_language(C CXX)

#自定义变量 环境选项
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-rtti -std=c++17 -g -Wall")
# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -fno-rtti -std=c++17 -g -Wall")

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)   #可执行文件目录
set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)      #库输出路径
include_directories(
    ## 下面两个都 include ，为了兼容 vscode cmake tools
    ${PROJECT_BINARY_DIR}/rpc/proto     
    ${PROJECT_BINARY_DIR}/../rpc/proto
)
add_definitions(-Dcmakebuild) 




#设置输出路径
set(MESSAGE_DIR ${CMAKE_BINARY_DIR}/rpc/proto)
if(EXISTS "${CMAKE_BINARY_DIR}/rpc/proto" AND IS_DIRECTORY "${CMAKE_BINARY_DIR}/rpc/proto")
        SET(DST_DIR ${MESSAGE_DIR})
else()
        file(MAKE_DIRECTORY ${MESSAGE_DIR})
        SET(DST_DIR ${MESSAGE_DIR})
endif()

#设置protoc的搜索路径
LIST(APPEND PROTO_FLAGS -I${CMAKE_CURRENT_SOURCE_DIR})

#获取需要编译的proto文件
file(GLOB MSG_PROTOS ${CMAKE_BINARY_DIR}/message/*.proto)   # 搜索所有proto文件

# message("proto搜索结果："  ${MSG_PROTOS})
set(MESSAGE_SRC "")
set(MESSAGE_HDRS "")
foreach(msg ${MSG_PROTOS})
        get_filename_component(FIL_WE ${msg} NAME_WE)

        list(APPEND MESSAGE_SRC "${PROJECT_BINARY_DIR}/rpc/proto${FIL_WE}.pb.cc")
        list(APPEND MESSAGE_HDRS "${PROJECT_BINARY_DIR}/rpc/proto${FIL_WE}.pb.h")
        
        # 生成源码
        execute_process(
            COMMAND ${PROTOBUF_PROTOC_EXECUTABLE} ${PROTO_FLAGS} --cpp_out=${DST_DIR}  ${msg}
            )
        message(${msg})
endforeach()
set_source_files_properties(${MESSAGE_SRC} ${MESSAGE_HDRS} PROPERTIES GENERATED TRUE)


file(GLOB SRC_FILE 
    rpc/YRoutine/*.cc
    #${PROJECT_BINARY_DIR}/rpc/YRoutine/*.S  # 暂时没啥用了，安心用boost context
    rpc/Util/*.cc
    rpc/msg/*.cc
    rpc/network/*.cc
    rpc/protocol/*.cc
    rpc/proto/*/*.cc        # protobuf 生成头文件
    rpc/yrpc/*.cc
)


add_library(yrpc SHARED ${SRC_FILE})


#protobuf lib 依赖
find_package(Protobuf REQUIRED)
if(PROTOBUF_FOUND)
    message(STATUS "protobuf library found")
else()
    message(FATAL_ERROR "protobuf library is needed but cant be found")
endif()


target_link_libraries(yrpc   ${PROTOBUF_LIBRARIES} boost_context pthread)

install(TARGETS yrpc DESTINATION lib)

install(FILES ${INCLUDE_FILE} DESTINATION include)


add_subdirectory(rpc)



