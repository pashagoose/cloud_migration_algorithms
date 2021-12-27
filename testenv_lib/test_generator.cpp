#include "test_generator.h"

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

Problem Generate() {
	// TODO
	
}