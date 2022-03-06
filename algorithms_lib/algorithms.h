#include "../testenv_lib/solution.h"

#include <cassert>
#include <map>
#include <optional>
#include <queue>
#include <vector>

#include "parallelizer.h"

namespace AlgoBaseline {
	std::optional<Solution> Solve(const Problem& problem);
}

namespace AlgoParallelBaseline {
	std::optional<Solution> Solve(const Problem& problem);
}

namespace AlgoFlowGrouping {
	std::optional<Solution> Solve(const Problem& problem);
}