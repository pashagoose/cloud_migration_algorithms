#include <iostream>

#include <glog/logging.h>

#include "../algorithms_lib/algorithms.h"
#include "../testenv_lib/test_environment.h"

int main(int argc, const char* argv[]) {
	FLAGS_logtostderr = true;
    google::InitGoogleLogging(argv[0]);
    google::InstallFailureSignalHandler();

    if (argc < 3) {
    	std::cout << "USAGE: ./count_lowerbound <proto_test_chunk_path> <result_path>\n";
    	return 1;
    }
    
    DataSet::DataSet dataset = LoadTests(argv[1]);
    std::ofstream fout(argv[2], std::ios::out | std::ios::trunc);

    for (size_t i = 0; i < dataset.tests_size(); ++i) {
        Problem problem = ConvertTestCaseToProblem(dataset.tests(i));
        long double lowerbound = AlgoLowerBound::CountTimespanLowerBound(problem);
        fout << lowerbound << "\n";
    }

	return 0;
}