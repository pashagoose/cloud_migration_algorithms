#include "test_environment.h"
#include <stdexcept>

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
}

void Server::SendVM(const VM& vm) {
	if (!free_upload_connections_) {
		throw std::runtime_error("Server #" + std::to_string(id_) + " cannot send so many VMs");
	}

	free_mem_ += vm.mem;
	free_cpu_ += vm.cpu;
	--free_upload_connections_;
}

void Server::CancelReceivingVM() {
	++free_download_connections_;
}

void Server::CancelSendingVM() {
	++free_upload_connections_;
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
		for (const auto& move : vm_moves) {
			movements.emplace_back(move);
		}
	}

	std::sort(movements.begin(), movements.end(), [&]
		(const Movement& lhs, const Movement& rhs) 
	{	
		return lhs.start_moment < rhs.start_moment; 
	});

	// Emulate



	// Check equality of final configurations


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