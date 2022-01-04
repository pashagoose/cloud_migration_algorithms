#include "../testenv_lib/solution.h"

namespace AlgoBaseline {

Solution Solve(const Problem& problem) {
	long double timer = 0;
	Solution solution;

	std::vector<size_t> vm_pos = problem.start_position.vm_server;
	std::vector<std::set<size_t>> servers(problem.server_specs.size());

	// init

	for (const auto& vm : problem.vms) {
		servers[vm_pos[vm.id]].insert(vm.id);
	}

	// perform consequtive moves using buffer server

	/*for (size_t i = 0; i < problem.vms.size(); ++i) {
		size_t dest = problem.end_position.vm_server[i];

		if (vm_pos[i] == dest) {
			continue;
		}

		
	}*/

	return solution;
}

}