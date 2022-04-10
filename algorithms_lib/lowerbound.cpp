#include "algorithms.h"

namespace AlgoLowerBound {

long double CountTimespanLowerBound(const Problem& problem) {
	long double res = 0;

	std::vector<std::vector<VM>> moving_in_vms(problem.server_specs.size());
	std::vector<std::vector<VM>> moving_out_vms(problem.server_specs.size());

	for (const VM& vm : problem.vms) {
		if (problem.end_position.vm_server[vm.id] 
			!= problem.start_position.vm_server[vm.id]) {
			moving_in_vms[problem.end_position.vm_server[vm.id]].push_back(vm);
			moving_out_vms[problem.start_position.vm_server[vm.id]].push_back(vm);
		}
	}

	auto sort_by_migration_time = [](const VM& lhs, const VM& rhs) -> bool {
		return lhs.migration_time >= rhs.migration_time;
	};

	auto count_lowerbound = [](const std::vector<VM>& vms, size_t round_capacity) -> long double {
		long double timespan_lowerbound = 0;
		for (size_t j = 0; j < vms.size(); ++j) {
			if (j % round_capacity + 1 == round_capacity
				|| j + 1 == vms.size()) {
				timespan_lowerbound += vms[j].migration_time;
			}
		}
		return timespan_lowerbound;
	};

	for (size_t i = 0; i < moving_in_vms.size(); ++i) {
		std::vector<VM>& vms = moving_in_vms[i];
		std::sort(vms.begin(), vms.end(), sort_by_migration_time);
		res = std::max(res, count_lowerbound(vms, problem.server_specs[i].max_in));
	}

	for (size_t i = 0; i < moving_out_vms.size(); ++i) {
		std::vector<VM>& vms = moving_out_vms[i];
		std::sort(vms.begin(), vms.end(), sort_by_migration_time);
		res = std::max(res, count_lowerbound(vms, problem.server_specs[i].max_out));
	}

	return res;
}

}