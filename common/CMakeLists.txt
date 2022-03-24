include_directories(${CMAKE_CURRENT_BINARY_DIR})

set(COMMON_SRCS metrics.cpp solution.cpp)

add_library(common_lib STATIC ${COMMON_SRCS})

target_link_libraries(common_lib PUBLIC
	glog::glog)

get_target_property(GLOG_INCLUDES glog::glog INCLUDE_DIRECTORIES)
target_include_directories(common_lib SYSTEM PUBLIC ${GLOG_INCLUDES})
