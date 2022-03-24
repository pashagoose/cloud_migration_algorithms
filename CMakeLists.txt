cmake_minimum_required(VERSION 3.10)

project(cloud_migration_algorithms)

if (ASAN)
    message("Building with address sanitizer")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fsanitize=address -fsanitize-address-use-after-scope -fno-omit-frame-pointer")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -fsanitize-address-use-after-scope -fno-omit-frame-pointer")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=address")
endif()

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

include(FetchContent)
include(cmake/FindGlog.cmake)
include(FindProtobuf)

find_package(Protobuf REQUIRED)

if(NOT PROTOBUF_FOUND)
    message(SEND_ERROR "Failed to find PROTOBUF")
    return()
else()
    message("Found PROTOBUF ${PROTOBUF_LIBRARY}")
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_compile_options(-Wall -Wextra -pedantic)

add_subdirectory(algorithms_lib)
add_subdirectory(testenv_lib)
add_subdirectory(benchmarks)
add_subdirectory(proto)
add_subdirectory(test_dumper)
add_subdirectory(common)
