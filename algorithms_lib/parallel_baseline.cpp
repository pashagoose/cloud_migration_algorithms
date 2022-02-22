#include "algorithms.h"

#include <glog/logging.h>

namespace AlgoParallelBaseline {

std::optional<Solution> Solve(const Problem& problem) {
	std::optional<Solution> res = AlgoBaseline::Solve(problem);

	if (!res) {
		return res;
	} 

	std::vector<Movement> moves;
	for (const auto& vm_moves : res->vm_movements) {
		for (const auto& move : vm_moves) {
			moves.push_back(move);
		}
	}

	std::sort(moves.begin(), moves.end(), 
		[&](const Movement& lhs, const Movement& rhs) 
	{
		return lhs.start_moment < rhs.start_moment;
	});

	std::multimap<long double, Movement> migrations; // migrations are sorted by end times

	std::vector<Server> servers;
	servers.reserve(problem.server_specs.size());

	for (size_t i = 0; i < problem.server_specs.size(); ++i) {
		servers.emplace_back(problem.server_specs[i], i);
	}

	for (const auto& vm : problem.vms) {
		servers[problem.start_position.vm_server[vm.id]].ReceiveVM(vm);
		servers[problem.start_position.vm_server[vm.id]].CancelReceivingVM(vm);
	}

	Solution new_solution(res->vm_movements.size());

	long double timer = 0;
	size_t ptr = 0;

	auto add_new_migrations_to_solution = [&]() {
		while (ptr < moves.size()) {
			auto move = moves[ptr];

			if (servers[move.from].CanSendVM() && servers[move.to].CanReceiveVM(problem.vms[move.vm_id])) {
				servers[move.from].SendVM(problem.vms[move.vm_id]);
				servers[move.to].ReceiveVM(problem.vms[move.vm_id]);
				migrations.insert({timer + move.duration, move});
				new_solution.vm_movements[move.vm_id].push_back(
					Movement{
						.from = move.from,
						.to = move.to,
						.start_moment = timer,
						.duration = move.duration,
						.vm_id = move.vm_id
					}
				);
				++ptr;
			} else {
				break;
			}
		}
	};

	add_new_migrations_to_solution();

	while (!migrations.empty()) {
		auto [moment, move] = *(migrations.begin());
		migrations.erase(migrations.begin());

		servers[move.from].CancelSendingVM(problem.vms[move.vm_id]);
		servers[move.to].CancelReceivingVM(problem.vms[move.vm_id]);

		timer = moment;
		add_new_migrations_to_solution();
	}

	return new_solution;
}

}