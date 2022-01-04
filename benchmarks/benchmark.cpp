#include <iostream>

#include <glog/logging.h>

#include "../testenv_lib/test_environment.h"
#include "../testenv_lib/test_generator.h"

int main(int argc, const char* argv[]) {
	FLAGS_logtostderr = true;
    google::InitGoogleLogging(argv[0]);
    google::InstallFailureSignalHandler();

	LOG(INFO) << "bench stub: " << argv[0];

	RealLifeGenerator gen(137, 15, 3, 3);

	for (size_t i = 0; i < 15; ++i) {
		Problem prob = gen.Generate();
		PrintTest(prob);
	}
	
	return 0;
}