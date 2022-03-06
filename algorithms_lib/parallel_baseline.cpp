#include "algorithms.h"

#include <glog/logging.h>

namespace AlgoParallelBaseline {

std::optional<Solution> Solve(const Problem& problem) {
	return Parallelizer::ParallelizeSolution(AlgoBaseline::Solve, problem);
}

}