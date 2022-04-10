include_directories(${CMAKE_CURRENT_BINARY_DIR})

add_executable(benchmark benchmark.cpp)
add_executable(count_lowerbound count_lowerbound.cpp)

target_link_libraries(benchmark testenv_lib algorithms_lib proto_lib)
target_link_libraries(count_lowerbound testenv_lib algorithms_lib proto_lib)
