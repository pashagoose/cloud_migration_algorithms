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

long double SumMigrationTime::Evaluate(const Problem& task, const Solution& solution) {
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