#include "../testenv_lib/solution.h"

#include <map>
#include <optional>
#include <vector>

namespace AlgoBaseline {
	std::optional<Solution> Solve(const Problem& problem);
}

namespace AlgoParallelBaseline {
	std::optional<Solution> Solve(const Problem& problem);
}

namespace AlgoFlowGrouping {
	std::optional<Solution> Solve(const Problem& problem);
}