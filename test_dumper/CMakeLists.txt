include_directories(${CMAKE_CURRENT_BINARY_DIR})

add_executable(test_dumper test_dumper.cpp)

target_link_libraries(test_dumper testenv_lib algorithms_lib proto_lib common_lib)
