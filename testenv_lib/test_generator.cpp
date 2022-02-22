#include "test_generator.h"

#include <stdexcept>
#include <glog/logging.h>

RealLifeGenerator::RealLifeGenerator(
	size_t seed,
	size_t diff_percentage_max,
	size_t servers_quantity_min,
	size_t servers_quantity_max,
	std::vector<std::pair<size_t, size_t>>&& ratios
)
	: diff_percentage_max_(diff_percentage_max)
	, servers_quantity_min_(servers_quantity_min)
	, servers_quantity_max_(servers_quantity_max)
	, ratios_(ratios)
	, rnd_(seed)
{
}

void PrintVMArrangement(
	const Problem& problem,
	const VMArrangement& arrangement
) {
	std::vector<std::vector<size_t>> server_vms(problem.server_specs.size());

	for (size_t i = 0; i < problem.vms.size(); ++i) {
		server_vms[arrangement.vm_server[i]].push_back(i);
	}

	for (size_t i = 0; i < server_vms.size(); ++i) {
		std::cerr << "Server #" << i << " vCPU: " << problem.server_specs[i].cpu
			<< " RAM: " << problem.server_specs[i].mem << '\n';

		for (auto vm_id : server_vms[i]) {
			std::cerr << "VM #" << vm_id << " vCPU: " << problem.vms[vm_id].cpu
				<< " RAM: " << problem.vms[vm_id].mem << '\n';
		}
	}
} 

void PrintTest(const Problem& problem) {
	std::cout << "Servers quantity: " << problem.server_specs.size() << '\n';

	std::cerr << "Start arrangement\n";
	PrintVMArrangement(problem, problem.start_position);

	std::cerr << "\nEnd arrangement\n";
	PrintVMArrangement(problem, problem.end_position);
}

int RandomIntFromRange(int l, int r, std::mt19937& gen) {
	return l + (gen() % (r - l + 1));
}

Problem RealLifeGenerator::Generate() {
	size_t server_count = RandomIntFromRange(
		servers_quantity_min_,
		servers_quantity_max_,
		rnd_
	);

	constexpr size_t kMaximumVMsOnServer = 30;
	const std::vector<size_t> cpu_vm_variants = {1, 2, 4, 8, 16, 32, 64};
	const std::vector<double> cpu_vm_prob_weights = {
		1.0 / 8, 1.0 / 8, 1.0 / 4, 1.0 / 4, 1.0 / 8, 1.0 / 16, 1.0 / 16
	};
	const std::vector<ServerSpec> server_variants = {
		ServerSpec{256, 128, 1, 1},
		ServerSpec{128, 64, 1, 1},
		ServerSpec{512, 128, 1, 1},
		ServerSpec{256, 64, 1, 1},
		ServerSpec{512, 64, 1, 1}
	};

	constexpr double EPS = 1e-13;

	Problem result;
	result.server_specs.resize(server_count);
	size_t vm_count = 0;

	std::uniform_real_distribution<double> unif_gen(0, 1.0); 

	auto get_random_server_spec = [&]() -> ServerSpec {
		size_t idx = RandomIntFromRange(0, server_variants.size() - 1, rnd_);
		return server_variants[idx];
	};

	auto get_random_vm = [&](size_t freecpu, size_t freemem) -> std::optional<VM> {
		int tries = 6;
		while (tries--) {
			double rand_num = unif_gen(rnd_);
			size_t mem_multiplier = ratios_[RandomIntFromRange(0, ratios_.size() - 1, rnd_)].second;

			for (size_t i = 0; i < cpu_vm_prob_weights.size(); ++i) {
				if (rand_num >= cpu_vm_prob_weights[i] + EPS) {
					rand_num -= cpu_vm_prob_weights[i];
				} else {
					size_t mem = cpu_vm_variants[i] * mem_multiplier;
					size_t cpu = cpu_vm_variants[i];

					if (mem > freemem || cpu > freecpu) {
						break;
					}

					double mem_mig_velocity = 1.0 + unif_gen(rnd_);

					return std::optional<VM>(VM{
						cpu, 
						mem,
						vm_count++,
						mem_mig_velocity * static_cast<long double>(mem)
						// migration time equals to the memory size of VM for now
					});
				}
			}
		}

		return std::nullopt;
	};

	// Generate starting position

	for (size_t i = 0; i < result.server_specs.size(); ++i) {
		size_t vm_qty = RandomIntFromRange(0, kMaximumVMsOnServer, rnd_); // not quite it
		result.server_specs[i] = get_random_server_spec();

		size_t freemem = result.server_specs[i].mem;
		size_t freecpu = result.server_specs[i].cpu;

		while (vm_qty--) {
			std::optional<VM> new_vm = get_random_vm(freecpu, freemem);
			if (!new_vm) {
				break;
			}

			freecpu -= new_vm->cpu;
			freemem -= new_vm->mem;

			result.vms.push_back(*new_vm);
			result.start_position.vm_server.push_back(i);
		}
	}

	result.end_position = result.start_position;

	std::vector<VM> vms_for_move = result.vms;

	std::shuffle(vms_for_move.begin(), vms_for_move.end(), rnd_);

	auto get_random_server_spec_with_enough_space = [&](size_t cpu, size_t mem) {
		while (true) {
			auto spec = get_random_server_spec();
			if (spec.cpu >= cpu && spec.mem >= mem) {
				return spec;
			}
		}
	};

	size_t server_index = 0;
	size_t vms_to_move =  vms_for_move.size() * (static_cast<double>(diff_percentage_max_) / 100.0);

	std::vector<size_t> server_permutation(server_count);
	for (size_t i = 0; i < server_count; ++i) {
		server_permutation[i] = i;
	}
	std::shuffle(server_permutation.begin(), server_permutation.end(), rnd_);

	std::vector<Server> servers_emulation;
	servers_emulation.reserve(server_count);

	for (size_t i = 0; i < result.server_specs.size(); ++i) {
		servers_emulation.emplace_back(result.server_specs[i], i);
	}

	for (size_t i = vms_to_move; i < vms_for_move.size(); ++i) {
		servers_emulation[result.start_position.vm_server[vms_for_move[i].id]].ReceiveVM(vms_for_move[i]);
		servers_emulation[result.start_position.vm_server[vms_for_move[i].id]].CancelReceivingVM(vms_for_move[i]);
	}

	for (size_t i = 0; i < std::min(vms_for_move.size(), vms_to_move); ++i) {
		const VM& vm = vms_for_move[i];

		size_t iters = 0;
		while (iters != result.server_specs.size()) {
			if (servers_emulation[server_permutation[server_index]].CanFit(vm)) {
				servers_emulation[server_permutation[server_index]].ReceiveVM(vm);
				servers_emulation[server_permutation[server_index]].CancelReceivingVM(vm);
				result.end_position.vm_server[vm.id] = server_permutation[server_index];
				break;
			} else {
				server_index = (server_index + 1) % result.server_specs.size();
				++iters;
			}
		}

		if (iters == result.server_specs.size()) {
			result.server_specs.push_back(
				get_random_server_spec_with_enough_space(vm.cpu, vm.mem)
			);
			server_permutation.push_back(result.server_specs.size() - 1);
			servers_emulation.emplace_back(result.server_specs.back(), result.server_specs.size() - 1);

			servers_emulation.back().ReceiveVM(vm);
			servers_emulation.back().CancelReceivingVM(vm);

			result.end_position.vm_server[vm.id] = result.server_specs.size() - 1;
			server_index = result.server_specs.size() - 1;
		}
	}

	// serves as buffer
	result.server_specs.push_back(ServerSpec{512, 128, 1, 1});

	return result;
}