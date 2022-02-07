#include <iostream>

#include <glog/logging.h>

#include "../testenv_lib/test_environment.h"
#include "../testenv_lib/test_generator.h"
#include "../algorithms_lib/algorithms.h"

int main(int argc, const char* argv[]) {
	FLAGS_logtostderr = true;
    google::InitGoogleLogging(argv[0]);
    google::InstallFailureSignalHandler();

    constexpr size_t tests = 1;

	TestEnvironment test_env(42, 15, 3, 5);
	size_t solved = test_env.RunTests(tests, AlgoBaseline::Solve);

	LOG(INFO) << "Solved " << solved << " cases out of " << tests << " tests";

	test_env.PrintMeasurements(std::cout);
	
	return 0;
}