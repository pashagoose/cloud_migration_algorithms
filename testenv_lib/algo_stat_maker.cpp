#include "algo_stat_maker.h"

void AlgoStatMaker::AddStat(const AlgoStat& stat) {
	stats_.push_back(stat);
} 

AlgoStat AlgoStatMaker::GetLastStat() const {
	if (stats_.empty()) {
		throw std::runtime_error("Algo stats is empty, cannot get last stat");
	}
	return stats_.back();
}