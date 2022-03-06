#include "algorithms.h"

#include <glog/logging.h>

namespace AlgoFlowGrouping {

constexpr size_t kMaximumLayers = 1e9;
constexpr size_t kMaximumFlow = 1e9;

struct Edge {
	size_t from;
	size_t to;
	size_t capacity;
	size_t id;
	bool rev;
};

struct Graph {
	std::vector<std::vector<Edge>> adjLists;
};

struct FlowState {
	Graph g;
	std::vector<size_t> flow; // array with flow values for each edge
	size_t totalFlow = 0;
	size_t sink;
	size_t drain;

	FlowState(const Graph& g_new, size_t s, size_t t) : g(g_new), sink(s), drain(t) {
		size_t edges_count = 0;
		for (size_t i = 0; i < g.adjLists.size(); ++i) {
			edges_count += g.adjLists[i].size();
		}

		flow.resize(edges_count / 2, 0); // each edge is counted twice (as straight and reverse)
	}

	size_t GetResidualThroughput(const Edge& edge) const {
		return edge.rev ? flow[edge.id] : edge.capacity - flow[edge.id];
	}
};

size_t GetResidualThroughput(const Edge& edge, const std::vector<size_t>& flow) {
	return edge.rev ? flow[edge.id] : edge.capacity - flow[edge.id];
}

size_t FindBlockingFlow(
	size_t v, 
	const Graph& g,
	size_t flow,
	std::vector<size_t>& adjPtrs,
	std::vector<size_t>& blockFlow,
	const std::vector<size_t>& distances
) 
{
	if (v == g.drain) {
		return flow;
	}

	while (adjPtrs[v] < g.adjLists[v].size()) {
		auto edge = g.adjLists[v][adjPtrs[v]++];
		size_t canPush = GetResidualThroughput(edge, blockFlow);
		if (canPush && distances[e.to] == distances[v] + 1) {
			size_t pushed = FindBlockingFlow(e.to, g, std::min(flow, canPush), adjPtrs, blockFlow, distances);

			if (pushed == 0) {
				continue;
			}

			if (e.rev) {
				blockFlow[e.id] -= pushed;
			} else {
				blockFlow[e.id] += pushed;
			}
			return pushed;
		}
	}

	return 0;
}

size_t DinicIteration(FlowState& flowState) {
	/*
		Return how much flow have been pushed

		1) Find layered network using bfs
		2) Find blocking flow through layered network and change residual network
	*/

	auto& g = flowState.g;
	auto& flow = flowState.flow;
	auto& totalFlow = flowState.totalFlow;

	// 1---------------------------------------------------

	std::vector<size_t> distances(g.size(), kMaximumLayers);
	distances[g.sink] = 0;
	std::queue<size_t> bfsQueue = {g.sink};

	while (!bfsQueue.empty()) {
		auto curv = bfsQueue.front();
		bfsQueue.pop();

		for (const auto& e : g.adjLists[curv]) {
			if (flowState.GetResidualThroughput(e) &&
				distances[e.to] > distances[curv] + 1) {
				distances[e.to] = distances[curv] + 1;
				bfsQueue.push(e.to);
			}
		}
	}

	// 2----------------------------------------------------

	std::vector<size_t> adjPtrs(g.size());

	size_t pushedTotal = 0;
	while (size_t pushed = FindBlockingFlow(g.s, g, kMaximumFlow, adjPtrs, flow, distances)) {
		pushedTotal += pushed;
	}

	if (pushedTotal == 0) {
		return false;
	}

	totalFlow += pushedTotal;

	return pushedTotal;
}

FlowState DinicFindMaxFlow(const Graph& g) {
	FlowState flowState(g);
	while (DinicIteration(flowState)) {}
	return flowState;
}

std::optional<Solution> Solve(const Problem& problem) {

}

}