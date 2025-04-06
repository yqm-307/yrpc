cmake_minimum_required(VERSION 3.2)

project(yrpc)

option(RELEASE
       "Release build (default: Debug)"
       OFF)

set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib) 

if (RELEASE)
    set(CMAKE_BUILD_TYPE Release)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -o3 -Wall")
else()
    set(CMAKE_BUILD_TYPE Debug)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -g -Wall")
endif()

include_directories(
    ${PROJECT_SOURCE_DIR}
)

file(GLOB_RECURSE SOURCE_FILES
    ${PROJECT_SOURCE_DIR}/bbt/*.cc
)

add_library(bbt_rpc SHARED
    ${SOURCE_FILES}
)

target_link_libraries(bbt_rpc PRIVATE
    inih
    bbt_core
    bbt_pollevent
    bbt_network
)

add_subdirectory(example)

add_subdirectory(benchmark)