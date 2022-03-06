add_library(algorithms_lib STATIC 
	baseline.cpp
	parallel_baseline.cpp
	flow_grouping.cpp
)

target_link_libraries(algorithms_lib PUBLIC
	glog::glog)

get_target_property(GLOG_INCLUDES glog::glog INCLUDE_DIRECTORIES)
target_include_directories(algorithms_lib SYSTEM PUBLIC ${GLOG_INCLUDES})
