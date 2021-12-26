#include "test_generator.h"

RealLifeGenerator::RealLifeGenerator(
	size_t seed,
	size_t diff_percentage_max = 15,
	size_t servers_quantity_min = 100,
	size_t servers_quantity_max = 5000,
	std::vector<std::pair<size_t, size_t>>&& ratios = {{1, 1}, {1, 2}, {1, 4}}
)
	: diff_percentage_max_(diff_percentage_max)
	, servers_quantity_min_(servers_quantity_min)
	, servers_quantity_max_(servers_quantity_max)
	, ratios_(ratios)
	, rnd_(seed)
{
}

std::pair<VMArrangement, VMArrangement> Generate() {
	// TODO
};