add_library(algorithms_lib STATIC 
	baseline.cpp
	parallel_baseline.cpp
	flow_grouping.cpp
	lowerbound.cpp
)

target_link_libraries(algorithms_lib PUBLIC
	glog::glog
	testenv_lib
	common_lib)

get_target_property(GLOG_INCLUDES glog::glog INCLUDE_DIRECTORIES)
target_include_directories(algorithms_lib SYSTEM PUBLIC ${GLOG_INCLUDES})
