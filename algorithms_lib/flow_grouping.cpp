#include "algorithms.h"

#include <glog/logging.h>

namespace AlgoFlowGrouping {

struct cmp_by_mem {
	bool operator()(const VM& lhs, const VM& rhs) const {
		if (lhs.mem != rhs.mem) {
			return lhs.mem > rhs.mem;
		}
		return lhs.id < rhs.id;
	}
};

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

std::optional<Solution> SolveImpl(const Problem& problem, AlgoStatMaker* statmaker) {
	// ONLY WORKS IF ALL servers' `max_in` ARE 1
	/*
		1) Build bipartite graph, where each server is respresented as two vertices in different parts.
			These two vertices mean input and output of server. Draw edges which represent possible VM migrations
			(from output of source to input of destination, if there is enough space on destination server).
		2) Extract concurrent migration groups as maximum flow in this bipartite graph. If no VM can move
			to their destination -  create a couple of migration groups to break the cycle (like in baseline algorithm).
		3) Try to combine groups - if finished migration in i-th group - try to start any possible migration 
			in (i + 1)-th group, do not wait for whole i-th group.
	*/

	size_t servers_cnt = problem.server_specs.size();

	std::vector<Server> servers;
	std::vector<Server> servers_end_pos;
	servers.reserve(servers_cnt);
	servers_end_pos.reserve(servers_cnt);

	for (size_t i = 0; i < servers_cnt; ++i) {
		servers.emplace_back(problem.server_specs[i], i);
		servers_end_pos.emplace_back(problem.server_specs[i], i);
	}

	for (size_t i = 0; i < problem.vms.size(); ++i) {
		servers[problem.start_position.vm_server[i]].ReceiveVM(problem.vms[i]);
		servers[problem.start_position.vm_server[i]].CancelReceivingVM(problem.vms[i]);

		servers_end_pos[problem.end_position.vm_server[i]].ReceiveVM(problem.vms[i]);
		servers_end_pos[problem.end_position.vm_server[i]].CancelReceivingVM(problem.vms[i]);
	}

	std::set<VM, cmp_by_mem> available_for_migration;
	std::set<VM, cmp_by_mem> misplaced_vms;
	std::vector<size_t> vm_pos = problem.start_position.vm_server;

	auto recalculate = [&]() {
		available_for_migration.clear();
		misplaced_vms.clear();

		for (size_t i = 0; i < problem.vms.size(); ++i) {
			if (problem.end_position.vm_server[i] != vm_pos[i]) {
				misplaced_vms.insert(problem.vms[i]);
				if (servers[problem.end_position.vm_server[i]].CanFit(problem.vms[i])) {
					available_for_migration.insert(problem.vms[i]);
				}
			}
		}
	};

	auto perform_move = [&](size_t vm_id, size_t from, size_t to) {
		vm_pos[vm_id] = to;

		if (from == to) {
			return;
		}

		servers[from].SendVM(problem.vms[vm_id]);
		servers[to].ReceiveVM(problem.vms[vm_id]);
		servers[from].CancelSendingVM(problem.vms[vm_id]);
		servers[to].CancelReceivingVM(problem.vms[vm_id]);

		for (auto vm_id : *servers_end_pos[from].GetRawVMSet()) {
			if (
				servers[from].CanFit(problem.vms[vm_id]) &&
				misplaced_vms.contains(problem.vms[vm_id])
			) {
				available_for_migration.insert(problem.vms[vm_id]);
			}
		}

		for (auto vm_id : *servers_end_pos[to].GetRawVMSet()) {
			if (
				available_for_migration.contains(problem.vms[vm_id]) &&
				!servers[to].CanFit(problem.vms[vm_id])
			) {
				available_for_migration.erase(problem.vms[vm_id]);
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

		for (const auto& vm : available_for_migration) {
			size_t from = vm_pos[vm.id], to = servers_cnt + problem.end_position.vm_server[vm.id];
			g.adjLists[from].push_back(Edge{from, to, 1, edges_count, false});
			g.adjLists[to].push_back(Edge{to, from, 1, edges_count++, true});

			edge_vm_bijection.push_back(vm.id);
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

	auto cmp_vm_ids_by_mem = [&](size_t lhs, size_t rhs) {
		if (problem.vms[lhs].mem != problem.vms[rhs].mem) {
			return problem.vms[lhs].mem > problem.vms[rhs].mem;
		}

		return lhs < rhs;
	};

	Solution solution(problem.vms.size());
	long double timer = 0;

	recalculate();

	while (!misplaced_vms.empty()) {
		if (available_for_migration.empty()) {
			// oops, break the cycle (usually get here when misplaced vms quantity is 2-10)
			VM move_vm = *misplaced_vms.rbegin();

			size_t dest_server = problem.end_position.vm_server[move_vm.id];

			std::set<size_t>& raw_vms = *servers[dest_server].GetRawVMSet();

			std::vector<size_t> vm_sorted_by_mem(raw_vms.begin(), raw_vms.end());
			std::sort(vm_sorted_by_mem.begin(), vm_sorted_by_mem.end(), cmp_vm_ids_by_mem);
			std::reverse(vm_sorted_by_mem.begin(), vm_sorted_by_mem.end());

			size_t ptr_servers = 0;

			for (auto vm_id : vm_sorted_by_mem) {
				if (dest_server == problem.end_position.vm_server[vm_id]) {
					continue;
				}
				// find buffer server
				size_t iters = 0;
				
				while (iters != servers.size()) {
					if (servers[ptr_servers].CanFit(problem.vms[vm_id]) && ptr_servers != dest_server) {
						perform_move(vm_id, dest_server, ptr_servers);

						solution.vm_movements[vm_id].push_back(
							Movement{
								.from = dest_server,
								.to = ptr_servers,
								.start_moment = timer,
								.duration = problem.vms[vm_id].migration_time,
								.vm_id = vm_id
							}
						);
						timer += problem.vms[vm_id].migration_time;

						break;
					} else {
						++iters;
						ptr_servers = (ptr_servers + 1) % servers.size();
					}
				}

				if (servers[dest_server].CanFit(move_vm)) {
					break;
				}

				if (iters == servers.size()) {
					return std::nullopt;
				}
			}
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

					available_for_migration.erase(problem.vms[vm_id]);
					misplaced_vms.erase(problem.vms[vm_id]);
					perform_move(vm_id, vm_pos[vm_id], problem.end_position.vm_server[vm_id]);
				}
			}
		}

		timer += maxMigtime;
	}

	return solution;
}

std::optional<Solution> Solve(const Problem& problem, AlgoStatMaker* statmaker) {
	return Parallelizer::ParallelizeSolution(SolveImpl, problem, statmaker);
}

}