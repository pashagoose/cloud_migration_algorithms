#include <iostream>

#include <glog/logging.h>

#include "../testenv_lib/test_environment.h"
#include "../testenv_lib/test_generator.h"
#include "../algorithms_lib/algorithms.h"

// PROTO
#include "../proto/test_case.pb.h"
#include "../proto/metrics.pb.h"

#include "../common/solution.h"

int main(int argc, const char* argv[]) {
	FLAGS_logtostderr = true;
    google::InitGoogleLogging(argv[0]);
    google::InstallFailureSignalHandler();

    if (argc < 2) {
    	std::cout << "Need path for dumping tests\n";
    	return 1;
    }

	TestEnvironment test_env(42, 15, 500, 1000);

	test_env.GenerateAndDumpTests(argv[1], 100, [&](const Problem& testCase) -> bool {
		return true;
	});
	
	return 0;
}