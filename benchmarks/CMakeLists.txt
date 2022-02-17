include_directories(${CMAKE_CURRENT_BINARY_DIR})

add_executable(benchmark benchmark.cpp)

target_link_libraries(benchmark testenv_lib algorithms_lib proto_lib ${PROTOBUF_LIBRARIES})