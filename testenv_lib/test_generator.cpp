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

					return std::optional<VM>(VM{
						cpu, 
						mem,
						vm_count++,
						static_cast<long double>(mem) 
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

	std::vector<VM> vms = result.vms;

	std::shuffle(vms.begin(), vms.end(), rnd_);

	size_t server_index = 0;
	size_t freemem = result.server_specs[0].mem;
	size_t freecpu = result.server_specs[0].cpu;

	auto get_random_server_spec_with_enough_space = [&](size_t cpu, size_t mem) {
		while (true) {
			auto spec = get_random_server_spec();
			if (spec.cpu >= cpu && spec.mem >= mem) {
				return spec;
			}
		}
	};

	for (auto& vm : vms) {
		while (true) {
			if (server_index == result.server_specs.size()) {
				LOG(INFO) << "Adding new server in test generator";
				result.server_specs.push_back(
					get_random_server_spec_with_enough_space(vm.cpu, vm.mem)
				);

				freemem = result.server_specs[server_index].mem;
				freecpu = result.server_specs[server_index].cpu;
			}

			if (freemem >= vm.mem && freecpu >= vm.cpu) {
				freemem -= vm.mem;
				freecpu -= vm.cpu;
				result.end_position.vm_server[vm.id] = server_index;
				break;
			} else {
				++server_index;
			}
		}
	}

	// serves as buffer
	result.server_specs.push_back(ServerSpec{512, 128, 1, 1}); 

	// my guess is that end position is worse vm arrangement, so we swap
	std::swap(result.start_position, result.end_position);

	return result;
}