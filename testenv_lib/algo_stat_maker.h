#pragma once

#include <vector>
#include <stdexcept>

struct AlgoStat {
	size_t brokenCycles = 0;
	size_t migrationsBreakingCycles = 0;
	size_t totalMigrations = 0;
	// TODO: measure working time here
};

class AlgoStatMaker {
public:
	AlgoStatMaker() = default;

	void AddStat(const AlgoStat& stat);

	AlgoStat GetLastStat() const;

private:	
	std::vector<AlgoStat> stats_;
};