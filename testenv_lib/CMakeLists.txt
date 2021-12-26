add_library(testenv_lib STATIC 
	grader.cpp
	metrics.cpp
	solution.cpp
	test_environment.cpp
	test_generator.cpp
)

target_link_libraries(testenv_lib PUBLIC
	glog::glog)

get_target_property(GLOG_INCLUDES glog::glog INCLUDE_DIRECTORIES)
target_include_directories(testenv_lib SYSTEM PUBLIC ${GLOG_INCLUDES})