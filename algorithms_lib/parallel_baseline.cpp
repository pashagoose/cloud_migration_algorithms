#include "algorithms.h"

#include <glog/logging.h>

namespace AlgoParallelBaseline {

std::optional<Solution> Solve(const Problem& problem, AlgoStatMaker* statmaker) {
	return Parallelizer::ParallelizeSolution(AlgoBaseline::Solve, problem, statmaker);
}

}