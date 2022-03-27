include_directories(${CMAKE_CURRENT_BINARY_DIR})

set(TESTENV_SRCS algo_stat_maker.cpp grader.cpp test_environment.cpp test_generator.cpp)

add_library(testenv_lib STATIC ${TESTENV_SRCS})

target_link_libraries(testenv_lib PUBLIC
	glog::glog
	proto_lib
	common_lib
)

get_target_property(GLOG_INCLUDES glog::glog INCLUDE_DIRECTORIES)
target_include_directories(testenv_lib SYSTEM PUBLIC ${GLOG_INCLUDES})
