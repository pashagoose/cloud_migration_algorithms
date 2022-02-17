#pragma once

#include <algorithm>
#include <memory>
#include <map>
#include <vector>

#include "metrics.h"
#include "test_generator.h"

class TestEnvironment {
public:
	TestEnvironment(
		size_t seed = 42,
		size_t diff_percentage_max = 15,
		size_t servers_quantity_min = 100,
		size_t servers_quantity_max = 1000
	);

	template<class Algorithm>
	size_t RunTests(size_t tests_count, Algorithm solver) {
		// Return solved_cases
		size_t solved_cases = 0;

		for (size_t i = 0; i < tests_count; ++i) {
			problem_ = generator_->Generate();
			solution_ = solver(problem_);

			//PrintTest(problem_);

			if (solution_) {
				CheckCorrectness();
				CountMetrics();
				++solved_cases;
			}
		}

		return solved_cases;
	}

	void PrintMeasurements(std::ostream& out) const;
	void ClearMeasurements();

private:
	void CheckCorrectness() const;
	void CountMetrics();

private:
	Problem problem_;
	std::optional<Solution> solution_;

	std::vector<std::unique_ptr<IMetric>> metrics_;
	std::vector<MetricsAccumulator> accumulators_;
	std::unique_ptr<ITestGenerator> generator_;
};