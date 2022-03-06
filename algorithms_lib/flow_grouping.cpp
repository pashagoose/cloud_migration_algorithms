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
	// ONLY WORKS IF ALL servers' `max_in` ARE 1
	/*
		1) Build bipartite graph, where each server is respresented as two vertices in different parts.
			These two vertices mean input and output of server. Draw edges which represent possible VM migrations
			(from output of source to input of destination, if there is enough space on destination server).
		2) Extract concurrent migration groups as maximum flow in this bipartite graph. If no VM can move
			to their destination - (TODO) create a couple of migration groups to break the cycle (like in baseline algorithm).
		3) (TODO) Try to combine groups - if finished migration in i-th group - try to start any possible migration 
			in (i + 1)-th group, do not wait for whole i-th group.
	*/

	size_t servers_cnt = problem.server_specs.size();

	std::vector<Server> servers;
	servers.reserve(servers_cnt);

	for (size_t i = 0; i < servers_cnt; ++i) {
		servers.emplace_back(problem.server_specs[i], i);
	}

	size_t misplaced = 0;

	for (size_t i = 0; i < problem.vms.size(); ++i) {
		misplaced += (problem.start_position.vm_server[i] != problem.end_position.vm_server[i]);
		servers[problem.start_position.vm_server[i]].ReceiveVM(problem.vms[i]);
		servers[problem.start_position.vm_server[i]].CancelReceivingVM(problem.vms[i]);
	}

	std::vector<size_t> available_for_migration;
	std::vector<size_t> vm_pos = problem.start_position.vm_server;

	auto recalculate = [&]() {
		available_for_migration.clear();
		misplaced = 0;

		for (size_t i = 0; i < problem.vms.size(); ++i) {
			if (problem.end_position.vm_server[i] != vm_pos[i]) {
				++misplaced;
				if (servers[problem.end_position.vm_server[i]].CanFit(problem.vms[i])) {
					available_for_migration.push_back(i);
				}
			}
		}
	};

	std::vector<size_t> edge_vm_bijection;

	auto build_graph = [&]() {
		Graph g;
		g.adjLists.resize(2 * servers_cnt + 2);
		g.sink = 2 * servers_cnt;
		g.drain = 2 * servers_cnt + 1;
		size_t edges_count = 0;
		edge_vm_bijection.clear();

		for (auto vm_id : available_for_migration) {
			size_t from = vm_pos[vm_id], to = servers_cnt + problem.end_position.vm_server[vm_id];
			g.adjLists[from].push_back(Edge{from, to, 1, edges_count, false});
			g.adjLists[to].push_back(Edge{to, from, 1, edges_count++, true});

			edge_vm_bijection.push_back(vm_id);
		}

		for (size_t i = 0; i < servers_cnt; ++i) {
			g.adjLists[g.sink].push_back(
				Edge{g.sink, i, problem.server_specs[i].max_out, edges_count, false}
			);
			g.adjLists[i].push_back(
				Edge{i, g.sink, problem.server_specs[i].max_out, edges_count++, true}
			);

			g.adjLists[servers_cnt + i].push_back(
				Edge{servers_cnt + i, g.drain, problem.server_specs[i].max_in, edges_count, false}
			);
			g.adjLists[g.drain].push_back(
				Edge{g.drain, servers_cnt + i, problem.server_specs[i].max_in, edges_count++, true}
			);
		}

		return g;
	};

	Solution solution(problem.vms.size());
	long double timer = 0;

	while (misplaced != 0) {
		LOG(INFO) << "Misplaced: " << misplaced;

		recalculate();

		if (available_for_migration.empty()) {
			// oops, break the cycle (usually get here when misplaced is about ~3)
			LOG(INFO) << "need to break";
			return std::nullopt;
		}

		Graph g = build_graph();
		FlowState maxflow = DinicFindMaxFlow(g);

		long double maxMigtime = 0;
		assert(maxflow.totalFlow != 0);

		for (size_t i = 0; i < servers_cnt; ++i) {
			for (const auto& e : g.adjLists[i]) {
				if (e.rev) continue;
				if (maxflow.flow[e.id] != 0) {
					size_t vm_id = edge_vm_bijection[e.id];
					
					maxMigtime = std::max(maxMigtime, problem.vms[vm_id].migration_time);
					solution.vm_movements[vm_id].push_back(
						Movement{
							.from = vm_pos[vm_id], 
							.to = problem.end_position.vm_server[vm_id],
							.start_moment = timer,
							.duration = problem.vms[vm_id].migration_time,
							.vm_id = vm_id
						}
					);

					servers[vm_pos[vm_id]].SendVM(problem.vms[vm_id]);
					servers[problem.end_position.vm_server[vm_id]].ReceiveVM(problem.vms[vm_id]);
					servers[problem.end_position.vm_server[vm_id]].CancelReceivingVM(problem.vms[vm_id]);
					servers[vm_pos[vm_id]].CancelSendingVM(problem.vms[vm_id]);

					vm_pos[vm_id] = problem.end_position.vm_server[vm_id];
				}
			}
		}

		timer += maxMigtime;
	}

	return solution;
}

}