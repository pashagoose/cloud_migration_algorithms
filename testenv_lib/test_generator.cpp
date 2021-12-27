#include "test_generator.h"
#include <random>

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
	size_t servers = RandomIntFromRange(
		servers_quantity_min_,
		servers_quantity_max_,
		rnd_
	);

	Problem result;
	result.server_specs.resize(servers);

	// TODO

	return result;
}