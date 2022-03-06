#include <iostream>

#include <glog/logging.h>

#include "../testenv_lib/test_environment.h"
#include "../testenv_lib/test_generator.h"
#include "../algorithms_lib/algorithms.h"

// PROTO
#include "../proto/test_case.pb.h"
#include "../proto/metrics.pb.h"


#include <google/protobuf/util/json_util.h>

int main(int argc, const char* argv[]) {
	FLAGS_logtostderr = true;
    google::InitGoogleLogging(argv[0]);
    google::InstallFailureSignalHandler();


    constexpr size_t tests = 100;

	TestEnvironment test_env(42, 15, 100, 1000);

	//DataSet::DataSet dataset = LoadTests(argv[1]);

	//Metrics::MetricsSet measurements = test_env.RunTestsFromDataSet(dataset, AlgoFlowGrouping::Solve);

	Metrics::MetricsSet measurements = test_env.RunTests(tests, AlgoFlowGrouping::Solve);

	LOG(INFO) << "Solved: " << measurements.solved() << " out of " << measurements.tests();

	test_env.PrintMeasurements(std::cout);
	//measurements.SerializeToOstream(&file);
	/*std::string result;
	google::protobuf::util::JsonPrintOptions options;
	options.add_whitespace = true;
	options.preserve_proto_field_names = true;

	google::protobuf::util::MessageToJsonString(measurements, &result, options);

	std::ofstream fout(argv[2], std::ios::binary | std::ios::trunc | std::ios::out);
	fout << result;
	*/
	return 0;
}