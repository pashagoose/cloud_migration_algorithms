#include "metrics.h"

#include <algorithm>

long double TotalTime::Evaluate(const Problem&, const Solution& solution) {
	long double result = 0;

	for (const auto& movements : solution.vm_movements) {
		for (const auto& movement : movements) {
			result = std::max(result, movement.start_moment + movement.duration);
		}
	}

	return result;
}

long double SumMigrationTime::Evaluate(const Problem&, const Solution& solution) {
	long double result = 0;

	for (const auto& movements : solution.vm_movements) {
		for (const auto& movement : movements) {
			result += movement.duration;
		}
	}

	return result;
}

long double TotalMemoryMigration::Evaluate(const Problem& task, const Solution& solution) {
	long double result = 0;

	for (const auto& movements : solution.vm_movements) {
		for (const auto& movement : movements) {
			result += task.vms[movement.vm_id].mem;
		}
	}

	return result;
}

MetricsAccumulator::MetricsAccumulator(std::string_view name)
	: metric_name_(name) {}

void MetricsAccumulator::Clear() {
	measurements_.clear();
}

void MetricsAccumulator::AppendMetric(long double val) {
	measurements_.push_back(val);
}

size_t MetricsAccumulator::TotalCount() const {
	return measurements_.size();
}

std::string MetricsAccumulator::GetName() const {
	return metric_name_;
}

long double MetricsAccumulator::GetMean() const {
	long double res = 0;
	long double divisor = TotalCount();

	for (long double val : measurements_) {
		res += val / divisor;
	}

	return res;
}
