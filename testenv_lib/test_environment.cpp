#include "test_environment.h"

TestEnvironment::TestEnvironment(size_t seed)
	: metrics_({
		std::make_unique<TotalTime>(),
		std::make_unique<TotalMemoryMigration>(),
		std::make_unique<SumMigrationTime>()
	})
	, measurements_(2, std::vector<long double>())
	, generator_(std::make_unique<RealLifeGenerator>(seed))
{
}

void TestEnvironment::CheckCorrectness() {
	// TODO
}

void TestEnvironment::CountMetrics() {
	for (size_t i = 0; i < metrics_.size(); ++i) {
		measurements_[i].push_back(metrics_[i]->Evaluate(problem_, solution_));
	}
}

void TestEnvironment::ClearMeasurements() {
	for (auto& metric_measurements : measurements_) {
		metric_measurements.clear();
	}
}	