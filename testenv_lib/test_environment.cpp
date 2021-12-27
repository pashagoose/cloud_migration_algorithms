#include "test_environment.h"

#include <stdexcept>
#include <glog/logging.h>

TestEnvironment::TestEnvironment(size_t seed)
	: metrics_({
		std::make_unique<TotalTime>(),
		std::make_unique<TotalMemoryMigration>(),
		std::make_unique<SumMigrationTime>()
	})
	, measurements_(2, std::vector<long double>())
	, generator_(std::make_unique<RealLifeGenerator>(seed))
{
}

Server::Server(const ServerSpec& spec, size_t id)
	: free_mem_(spec.mem)
	, free_cpu_(spec.cpu)
	, free_download_connections_(spec.max_in)
	, free_upload_connections_(spec.max_out)
	, id_(id)
	, spec_(spec)
{
}

void Server::ReceiveVM(const VM& vm) {
	if (free_mem_ < vm.mem) {
		throw std::runtime_error("Server #" + std::to_string(id_) + " has not enough memory for the move");
	}

	if (free_cpu_ < vm.cpu) {
		throw std::runtime_error("Server #" + std::to_string(id_) + " has not enough cpu for the move");
	}

	if (!free_download_connections_) {
		throw std::runtime_error("Server #" + std::to_string(id_) + " cannot receive so many VMs at one moment");
	}

	free_cpu_ -= vm.cpu;
	free_mem_ -= vm.mem;
	--free_download_connections_;

	vms_.insert(vm.id);
}

void Server::SendVM(const VM& vm) {
	if (!free_upload_connections_) {
		throw std::runtime_error("Server #" + std::to_string(id_) + " cannot send so many VMs");
	}

	free_mem_ += vm.mem;
	free_cpu_ += vm.cpu;
	--free_upload_connections_;

	vms_.erase(vm.id);
}

void Server::CancelReceivingVM() {
	++free_download_connections_;
}

void Server::CancelSendingVM() {
	++free_upload_connections_;
}

bool Server::HasVM(size_t vm_id) {
	return (vms_.contains(vm_id));
}
  
void TestEnvironment::CheckCorrectness() const {
	std::vector<Server> servers;
	servers.reserve(problem_.server_specs.size());

	for (size_t i = 0; i < problem_.server_specs.size(); ++i) {
		servers.emplace_back(problem_.server_specs[i], i);
	}

	// Fill start configuration

	for (size_t i = 0; i < problem_.start_position.vm_server.size(); ++i) {
		servers[problem_.start_position.vm_server[i]].ReceiveVM(problem_.vms[i]);
		servers[problem_.start_position.vm_server[i]].CancelReceivingVM();
	}

	// Sort all moves

	std::vector<Movement> movements;
	for (const auto& vm_moves : solution_.vm_movements) {
		long double prev_move_time = 0;
		for (const auto& move : vm_moves) {
			if (move.start_moment < prev_move_time) {
				throw std::runtime_error("Moves are intersecting for VM #" + std::to_string(move.vm_id));
			}

			movements.emplace_back(move);

			prev_move_time = move.start_moment + move.duration;
		}
	}

	std::sort(movements.begin(), movements.end(), [&]
		(const Movement& lhs, const Movement& rhs) 
	{	
		return lhs.start_moment < rhs.start_moment; 
	});

	// Emulate

	std::multimap<long double, Movement> transfer_endings;

	for (auto& move : movements) {
		// End passed transfers
		long double current_moment = move.start_moment;

		while (!transfer_endings.empty() && transfer_endings.begin()->first <= current_moment) {
			const auto& passed_move = transfer_endings.begin()->second;

			servers[passed_move.from].CancelSendingVM();
			servers[passed_move.to].CancelReceivingVM();

			transfer_endings.erase(transfer_endings.begin());
		}

		// Process current transfer
		servers[move.from].SendVM(problem_.vms[move.vm_id]);
		servers[move.to].ReceiveVM(problem_.vms[move.vm_id]);
		transfer_endings.insert({move.start_moment + move.duration, move});
	}

	// Check equality of the final configurations
	for (size_t i = 0; i < problem_.end_position.vm_server.size(); ++i) {
		size_t server_id = problem_.end_position.vm_server[i];

		if (!servers[server_id].HasVM(i)) {
			throw std::runtime_error("Result configuration is not equal to ending one: "
				 " no VM#" + std::to_string(i) + " on server#" + std::to_string(server_id));
		}
	}
}

void TestEnvironment::CountMetrics() {
	for (size_t i = 0; i < metrics_.size(); ++i) {
		measurements_[i].push_back(metrics_[i]->Evaluate(problem_, solution_));
	}
}

void TestEnvironment::ClearMeasurements() {
	for (auto& metric_measurements : measurements_) {
		metric_measurements.clear();
	}
}	