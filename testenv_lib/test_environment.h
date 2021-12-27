#pragma once

#include <algorithm>
#include <memory>
#include <map>
#include <vector>

#include "metrics.h"
#include "test_generator.h"

class Server {
public:
	Server(const ServerSpec& spec, size_t id);

	void ReceiveVM(const VM& vm);
	void SendVM(const VM& vm);

	void CancelReceivingVM();
	void CancelSendingVM();

	bool HasVM(size_t vm_id);

private:
	size_t free_mem_;
	size_t free_cpu_;
	size_t free_download_connections_;
	size_t free_upload_connections_;
	size_t id_;
	std::set<size_t> vms_;
	ServerSpec spec_;
};

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
	void CheckCorrectness() const;
	void CountMetrics();

private:
	Problem problem_;
	Solution solution_;

	std::vector<std::unique_ptr<IMetric>> metrics_;
	std::vector<std::vector<long double>> measurements_;
	std::unique_ptr<ITestGenerator> generator_;
};