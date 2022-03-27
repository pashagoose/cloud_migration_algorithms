#include "algorithms.h"

#include <glog/logging.h>

namespace AlgoBaseline {

struct cmp_by_mem {
	bool operator()(const VM& lhs, const VM& rhs) const {
		if (lhs.mem != rhs.mem) {
			return lhs.mem > rhs.mem;
		}
		return lhs.id < rhs.id;
	}
};

std::optional<Solution> Solve(const Problem& problem, AlgoStatMaker* statmaker) {
	/*
		M := (Set of misplaced VMs) is decreasing.
		Keep A := (set of misplaced VMs that can move to their destination right now).
		Take one VM, perform step, change A.
		if A is empty -> find minimal VM in M (in terms of memory), free some space for it
			by moving misplaced VMs from destination server to buffer, go greedy in order
			of increasing VMs' memory. Move this minimal VM.
	*/

	AlgoStat stats;

	long double timer = 0;
	Solution solution(problem.vms.size());

	std::vector<size_t> vm_pos = problem.start_position.vm_server;
	std::vector<Server> servers;
	std::vector<Server> servers_end_pos;
	servers.reserve(problem.server_specs.size());
	servers_end_pos.reserve(problem.server_specs.size());

	for (size_t i = 0; i < problem.server_specs.size(); ++i) {
		servers.emplace_back(problem.server_specs[i], i);
		servers_end_pos.emplace_back(problem.server_specs[i], i);
	}

	// init

	for (const auto& vm : problem.vms) {
		servers[vm_pos[vm.id]].ReceiveVM(vm);
		servers[vm_pos[vm.id]].CancelReceivingVM(vm);

		servers_end_pos[problem.end_position.vm_server[vm.id]].ReceiveVM(vm);
		servers_end_pos[problem.end_position.vm_server[vm.id]].CancelReceivingVM(vm);
	}

	auto cmp_vm_ids_by_mem = [&](size_t lhs, size_t rhs) {
		if (problem.vms[lhs].mem != problem.vms[rhs].mem) {
			return problem.vms[lhs].mem > problem.vms[rhs].mem;
		}

		return lhs < rhs;
	};

	std::set<VM, cmp_by_mem> misplaced_vms;
	std::set<VM, cmp_by_mem> available_for_migration;

	for (const auto& vm : problem.vms) {
		size_t end_pos = problem.end_position.vm_server[vm.id];

		if (end_pos != problem.start_position.vm_server[vm.id]) {
			misplaced_vms.insert(problem.vms[vm.id]);

			if (servers[end_pos].CanFit(problem.vms[vm.id])) {
				available_for_migration.insert(problem.vms[vm.id]);
			}
		}
	}

	auto perform_move = [&](size_t vm_id, size_t from, size_t to) {
		vm_pos[vm_id] = to;

		if (from == to) {
			return;
		}

		++stats.totalMigrations;

		servers[from].SendVM(problem.vms[vm_id]);
		servers[to].ReceiveVM(problem.vms[vm_id]);
		servers[from].CancelSendingVM(problem.vms[vm_id]);
		servers[to].CancelReceivingVM(problem.vms[vm_id]);

		solution.vm_movements[vm_id].push_back(Movement{
			.from = from,
			.to = to,
			.start_moment = timer,
			.duration = problem.vms[vm_id].migration_time,
			.vm_id = vm_id
		});

		timer += problem.vms[vm_id].migration_time;

		// recalculate available_for_migration

		for (auto vm_id : *servers_end_pos[from].GetRawVMSet()) {
			if (
				servers[from].CanFit(problem.vms[vm_id]) &&
				misplaced_vms.contains(problem.vms[vm_id])
			) {
				available_for_migration.insert(problem.vms[vm_id]);
			}
		}

		for (auto vm_id : *servers_end_pos[to].GetRawVMSet()) {
			if (
				available_for_migration.contains(problem.vms[vm_id]) &&
				!servers[to].CanFit(problem.vms[vm_id])
			) {
				available_for_migration.erase(problem.vms[vm_id]);
			}
		}
	};

	// perform consecutive moves using buffer server

	while (!misplaced_vms.empty()) {
		if (available_for_migration.empty()) {
			// break the cycle, use buffer
			VM move_vm = *misplaced_vms.rbegin();

			++stats.brokenCycles;

			size_t dest_server = problem.end_position.vm_server[move_vm.id];

			std::set<size_t>& raw_vms = *servers[dest_server].GetRawVMSet();

			std::vector<size_t> vm_sorted_by_mem(raw_vms.begin(), raw_vms.end());
			std::sort(vm_sorted_by_mem.begin(), vm_sorted_by_mem.end(), cmp_vm_ids_by_mem);
			std::reverse(vm_sorted_by_mem.begin(), vm_sorted_by_mem.end());

			size_t ptr_servers = 0;

			for (auto vm_id : vm_sorted_by_mem) {
				if (dest_server == problem.end_position.vm_server[vm_id]) {
					continue;
				}
				// find buffer server
				size_t iters = 0;
				while (iters != servers.size()) {
					if (servers[ptr_servers].CanFit(problem.vms[vm_id]) && ptr_servers != dest_server) {
						++stats.migrationsBreakingCycles;
						perform_move(vm_id, dest_server, ptr_servers);
						break;
					} else {
						++iters;
						ptr_servers = (ptr_servers + 1) % servers.size();
					}
				}

				if (!available_for_migration.empty()) {
					break;
				}

				if (iters == servers.size()) {
					if (statmaker) {
						statmaker->AddStat(stats);
					}
					return std::nullopt;
				}
			}

		} else {
			VM move_vm = *available_for_migration.begin();
			available_for_migration.erase(available_for_migration.begin());

			size_t from_server_id = vm_pos[move_vm.id];
			size_t to_server_id = problem.end_position.vm_server[move_vm.id];

			misplaced_vms.erase(move_vm);
			perform_move(move_vm.id, from_server_id, to_server_id);
		}
	}

	if (statmaker) {
		statmaker->AddStat(stats);
	}
	return solution;
}

}