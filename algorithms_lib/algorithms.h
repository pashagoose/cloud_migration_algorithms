#include "../common/solution.h"
#include "../testenv_lib/algo_stat_maker.h"

#include <cassert>
#include <map>
#include <optional>
#include <queue>
#include <vector>

#include "parallelizer.h"

namespace AlgoBaseline {
	std::optional<Solution> Solve(const Problem& problem, AlgoStatMaker* stats);
}

namespace AlgoParallelBaseline {
	std::optional<Solution> Solve(const Problem& problem, AlgoStatMaker* stats);
}

namespace AlgoFlowGrouping {
	std::optional<Solution> Solve(const Problem& problem, AlgoStatMaker* stats);
}

namespace AlgoLowerBound {
	long double CountTimespanLowerBound(const Problem& problem);
}
