#pragma once

#include <algorithm>
#include <iostream>
#include <optional>
#include <random>
#include <utility>

#include "../common/solution.h"

class ITestGenerator {
public:
	virtual Problem Generate() = 0;
};

void PrintTest(const Problem& problem);

class RealLifeGenerator final : public ITestGenerator { // TODO : checker for tests correctness
/*
	Not more than 15% difference between VM arrangements.
	Most popular VMs ratios - 1:1, 1:2, 1:4 (from 1 to 64 vCPUs).
	Servers quantity - from couple hundreds to couple thousands
*/
public:
	RealLifeGenerator(
		size_t seed,
		size_t diff_percentage_max = 15,
		size_t servers_quantity_min = 100,
		size_t servers_quantity_max = 1000,
		std::vector<std::pair<size_t, size_t>>&& ratios = {{1, 1}, {1, 2}, {1, 4}}
	);

	Problem Generate() override;

private:
	size_t diff_percentage_max_;
	size_t servers_quantity_min_;
	size_t servers_quantity_max_;
	std::vector<std::pair<size_t, size_t>> ratios_;
	std::mt19937 rnd_;// some random generator here
};