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
	size_t sink;
	size_t drain;
};

struct FlowState {
	Graph g;
	std::vector<size_t> flow; // array with flow values for each edge
	size_t totalFlow = 0;

	FlowState(const Graph& g_new) : g(g_new) {
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

size_t FindBlockingFlow(
	size_t v, 
	FlowState& flowState,
	size_t flow,
	std::vector<size_t>& adjPtrs,
	const std::vector<size_t>& distances
) 
{
	if (v == flowState.g.drain) {
		return flow;
	}

	while (adjPtrs[v] < flowState.g.adjLists[v].size()) {
		auto e = flowState.g.adjLists[v][adjPtrs[v]++];
		size_t canPush = flowState.GetResidualThroughput(e);

		if (canPush && distances[e.to] == distances[v] + 1) {
			size_t pushed = FindBlockingFlow(e.to, flowState, std::min(flow, canPush), adjPtrs, distances);

			if (pushed == 0) {
				continue;
			}

			if (e.rev) {
				flowState.flow[e.id] -= pushed;
			} else {
				flowState.flow[e.id] += pushed;
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

	// 1---------------------------------------------------

	std::vector<size_t> distances(g.adjLists.size(), kMaximumLayers);
	distances[g.sink] = 0;
	std::queue<size_t> bfsQueue;
	bfsQueue.push(g.sink);

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

	std::vector<size_t> adjPtrs(g.adjLists.size());

	size_t pushedTotal = 0;
	while (size_t pushed = FindBlockingFlow(g.sink, flowState, kMaximumFlow, adjPtrs, distances)) {
		pushedTotal += pushed;
	}

	if (pushedTotal == 0) {
		return false;
	}

	flowState.totalFlow += pushedTotal;

	return pushedTotal;
}

FlowState DinicFindMaxFlow(const Graph& g) {
	FlowState flowState(g);
	while (DinicIteration(flowState)) {}
	return flowState;
}

std::optional<Solution> Solve(const Problem& problem) {
	/*
		1) Build bipartite graph, where each server is respresented as two vertices in different parts.
			These two vertices mean input and output of server. Draw edges which represent possible VM migrations
			(from output of source to input of destination, if there is enough space on destination server).
		2) Extract concurrent migration groups as maximum flow in this bipartite graph. If no VM can move
			to their destination - break the cycle (as in baseline algorithm).
	*/

	throw std::runtime_error("Not implemented");
}

}