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

    if (argc < 4) {
    	std::cout << "USAGE: ./benchmark ALGO_NAME DATASET_INPUT_PATH METRICS_JSON_OUTPUT_PATH\n"; 
    	return 1;
    }

	TestEnvironment test_env(42, 15, 100, 1000);
	DataSet::DataSet dataset = LoadTests(argv[2]);
	TestEnvironment::AlgorithmCallback algo = AlgoBaseline::Solve;

	std::cout << "Using algorithm: `";
	if (std::string{argv[1]} == "flow_grouping") {
		std::cout << argv[1];
		algo = AlgoFlowGrouping::Solve;
	} else if (std::string{argv[1]} == "parallel_baseline") {
		std::cout << argv[1];
		algo = AlgoParallelBaseline::Solve;
	} else {
		std::cout << "baseline";
	}

	std::cout << "`\n";

// ------- Run ------------------------

	Metrics::MetricsSet measurements = test_env.RunTestsFromDataSet(dataset, algo, nullptr);
	LOG(INFO) << "Solved: " << measurements.solved() << " out of " << measurements.tests();

// ------------ Flush -----------------

	std::string result;
	google::protobuf::util::JsonPrintOptions options;
	options.add_whitespace = true;
	options.preserve_proto_field_names = true;

	google::protobuf::util::MessageToJsonString(measurements, &result, options);

	LOG(INFO) << "Flushing metrics to `" << argv[3] << "`";

	std::ofstream fout(argv[3], std::ios::binary | std::ios::trunc | std::ios::out);
	fout << result;
	
	return 0;
}