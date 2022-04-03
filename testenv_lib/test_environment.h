#pragma once

#include <algorithm>
#include <memory>
#include <map>
#include <vector>
#include <fstream>
#include <string>

#include "../common/metrics.h"
#include "test_generator.h"
#include "algo_stat_maker.h"

#include "../proto/test_case.pb.h"
#include "../proto/metrics.pb.h"

Problem ConvertTestCaseToProblem(const DataSet::TestCase& test);

DataSet::DataSet LoadTests(const std::string& path);

class TestEnvironment {
public:
	using TestPredicateCallback = std::function<bool(const Problem&)>;
	using AlgorithmCallback = std::function<std::optional<Solution>(const Problem&, AlgoStatMaker*)>;

	TestEnvironment(std::unique_ptr<ITestGenerator>&& test_generator);

	Metrics::MetricsSet RunTests(size_t tests_count, AlgorithmCallback solver, AlgoStatMaker* statmaker = nullptr);

	Metrics::MetricsSet RunTestsFromDataSet(DataSet::DataSet dataset, AlgorithmCallback solver, AlgoStatMaker* statmaker = nullptr);
	void GenerateAndDumpTests(const std::string& path, size_t test_count, TestPredicateCallback callback);

	// returns bool indicating where this problem can be solved by algorithm or not
	bool GetStatOnTest(const Problem& problem, AlgorithmCallback solver, AlgoStatMaker* statmaker);

	void PrintMeasurements(std::ostream& out) const;
	void ClearMeasurements();

private:

	void CheckCorrectness() const;
	void CountMetrics(Metrics::MetricsSet* measurements);

private:
	Problem problem_;
	std::optional<Solution> solution_;

	std::vector<std::unique_ptr<IMetric>> metrics_;
	std::vector<MetricsAccumulator> accumulators_;
	std::unique_ptr<ITestGenerator> generator_;
};