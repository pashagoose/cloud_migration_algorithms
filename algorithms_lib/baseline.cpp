#include "../testenv_lib/solution.h"

namespace AlgoBaseline {

Solution Solve(const Problem& problem) {
	/*
		M := (Set of misplaced VMs) is decreasing.
		Keep A := (set of misplaced VMs that can move to their destination right now).
		Take one VM, perform step, change A.
		if A is empty -> find minimal VM in M (in terms of memory), free some space for it
			by moving misplaced VMs from destination server to buffer, go greedy in order
			of increasing VMs' memory. Move this minimal VM.
	*/

	long double timer = 0;
	Solution solution;

	std::vector<size_t> vm_pos = problem.start_position.vm_server;
	std::vector<Server> servers(problem.server_specs.size());
	std::vector<Server> servers_end_pos(problem.server_specs.size());

	// init

	for (const auto& vm : problem.vms) {
		servers[vm_pos[vm.id]].ReceiveVM(vm);
		servers[vm_pos[vm.id]].CancelReceivingVM();

		servers_end_pos[problem.end_position.vm_server[vm.id]].ReceiveVM(vm);
		servers_end_pos[problem.end_position.vm_server[vm.id]].CancelReceivingVM();
	}

	auto cmp_by_mem = [&](size_t lhs, size_t rhs) {
		if (problem.vms[lhs].mem != problem.vms[rhs].mem) {
			return problem.vms[lhs].mem > problem.vms[rhs].mem;
		}

		return lhs < rhs;
	};

	set::set<size_t> misplaced_vms;
	std::set<size_t, cmp_by_mem> available_for_migration;

	for (const auto& vm : problem.vms) {
		size_t end_pos = problem.end_position.vm_server[vm.id];
		auto [cpu, mem] = servers[end_pos].GetFreeSpace();

		if (end_pos != problem.start_position.vm_server[vm.id]) {
			misplaced_vms.insert(vm.id);
		}

		if (cpu >= vm.cpu && mem >= vm.mem) {
			available_for_migration.insert(vm.id);
		}
	}

	// perform consequtive moves using buffer server

	while (!misplaced_vms.empty()) {
		if (available_for_migration.empty()) {
			// break the cycle, use buffer
		} else {
			size_t move_vm_id = *available_for_migration.begin();
			available_for_migration.erase(available_for_migration.begin());

			size_t from_server_id = vm_pos[move_vm_id];
			size_t to_server_id = problem.end_position.vm_server[move_vm_id];

			servers[from_server_id].SendVM(problem.vms[move_vm_id]);
			servers[from_server_id].CancelSendingVM();

			servers[to_server_id].ReceiveVM(problem.vms[move_vm_id]);
			servers[to_server_id].CancelReceivingVM(problem.vms[move_vm_id]);

			vm_pos[move_vm_id] = to_server_id;
			misplaced_vms.erase(move_vm_id);

			// recalculate available_for_migration

			for (auto vm_id : *servers_end_pos[from_server_id].GetRawVMSet()) {
				if (
					servers[from_server_id].CanFit(problem.vms[vm_id]) &&
					misplaced_vms.contains(vm_id)
				) {
					available_for_migration.insert(vm_id);
				}
			}

			for (auto vm_id : *server_end_pos[to_server_id].GetRawVMSet()) {
				if (
					available_for_migration.contains(vm_id) &&
					!servers[to_server_id].CanFit(problem.vms[vm_id])
				) {
					available_for_migration.erase(vm_id);
				}
			}
		}
	}

	return solution;
}

}