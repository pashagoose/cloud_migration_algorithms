#include <iostream>

#include <glog/logging.h>

#include "../testenv_lib/test_environment.h"
#include "../testenv_lib/test_generator.h"
#include "../algorithms_lib/algorithms.h"

// PROTO
#include "../proto/test_case.pb.h"

int main(int argc, const char* argv[]) {
	FLAGS_logtostderr = true;
    google::InitGoogleLogging(argv[0]);
    google::InstallFailureSignalHandler();

    if (argc < 2) {
    	std::cerr << "No path for tests\n";
    	return 0;
    }

    constexpr size_t tests = 100;

	TestEnvironment test_env(42, 15, 100, 1000);
	
	//test_env.GenerateAndDumpTests(argv[1], tests);

	DataSet::DataSet dataset = LoadTests(argv[1]);

	size_t solved = test_env.RunTests(tests, AlgoParallelBaseline::Solve);

	LOG(INFO) << "Solved: " << solved << " out of " << tests;
	
	return 0;
}