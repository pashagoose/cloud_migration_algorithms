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

    if (argc < 3) {
    	std::cout << "USAGE: ./test_dumper GENERATOR_TYPE OUTPUT_TESTS_PATH";
    	return 1;
    }


	TestEnvironment test_env(
		std::string{argv[1]} == "cycles" ? 
		std::unique_ptr<ITestGenerator>(std::make_unique<CyclesGenerator>(147, 25, 15, 500, 1000)) : 
		std::unique_ptr<ITestGenerator>(std::make_unique<RealLifeGenerator>(147, 25, 500, 1000))
	);

	AlgoStatMaker statmaker;

	test_env.GenerateAndDumpTests(argv[2], 100, [&](const Problem& testCase) -> bool {
		test_env.GetStatOnTest(testCase, AlgoBaseline::Solve, &statmaker);
		 /*AlgoStat lastStat = statmaker.GetLastStat();

		 if (lastStat.brokenCycles >= 2) {
		 	LOG(INFO) << "Adding test with stat: " << lastStat.brokenCycles << " "
		 		<< lastStat.migrationsBreakingCycles << ' ' << lastStat.totalMigrations;
		 	return true;
		 }*/

		 return true;
	});
	
	return 0;
}