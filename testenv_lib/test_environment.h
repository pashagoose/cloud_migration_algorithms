#pragma once

#include <memory>
#include <vector>

#include "metrics.h"
#include "test_generator.h"

class TestEnvironment {
public:
	TestEnvironment(size_t seed = 42);

	template<class Algorithm>
	void RunTests(size_t tests_count, Algorithm solver) {
		for (size_t i = 0; i < tests_count; ++i) {
			problem_ = generator_->Generate();
			solution_ = solver(problem_);

			CheckCorrectness();
			CountMetrics();
		}
	}

	void ClearMeasurements();

private:
	void CheckCorrectness();
	void CountMetrics();

private:
	Problem problem_;
	Solution solution_;

	std::vector<std::unique_ptr<IMetric>> metrics_;
	std::vector<std::vector<long double>> measurements_;
	std::unique_ptr<ITestGenerator> generator_;
};