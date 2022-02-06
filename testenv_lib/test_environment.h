#pragma once

#include <algorithm>
#include <memory>
#include <map>
#include <vector>

#include "metrics.h"
#include "test_generator.h"

class TestEnvironment {
public:
	TestEnvironment(size_t seed = 42);

	template<class Algorithm>
	void RunTests(size_t tests_count, Algorithm solver) {
		size_t solved_cases = 0;
		for (size_t i = 0; i < tests_count; ++i) {
			problem_ = generator_->Generate();
			solution_ = solver(problem_);

			if (solution_) {
				CheckCorrectness();
				CountMetrics();
				++solved_cases;
			}
		}
	}

	void ClearMeasurements();

private:
	void CheckCorrectness() const;
	void CountMetrics();

private:
	Problem problem_;
	std::optional<Solution> solution_;

	std::vector<std::unique_ptr<IMetric>> metrics_;
	std::vector<std::vector<long double>> measurements_;
	std::unique_ptr<ITestGenerator> generator_;
};