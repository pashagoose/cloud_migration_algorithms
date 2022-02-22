#include "solution.h"

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

	--free_upload_connections_;

	if (vms_.find(vm.id) == vms_.end()) {
		throw std::runtime_error("No VM#" + std::to_string(vm.id) + " on server");
	}
}

void Server::CancelReceivingVM(const VM& vm) {
	vms_.insert(vm.id);
	++free_download_connections_;
}

void Server::CancelSendingVM(const VM& vm) {
	free_mem_ += vm.mem;
	free_cpu_ += vm.cpu;
	vms_.erase(vm.id);
	++free_upload_connections_;
}

bool Server::HasVM(size_t vm_id) const {
	return (vms_.contains(vm_id));
}

std::set<size_t>* Server::GetRawVMSet() {
	return &vms_;
}

std::tuple<size_t, size_t> Server::GetFreeSpace() const {
	return {free_cpu_, free_mem_};
}

bool Server::CanFit(const VM& vm) const {
	return vm.mem <= free_mem_ && vm.cpu <= free_cpu_;
}

bool Server::CanSendVM() const {
	return free_upload_connections_;
}

bool Server::CanReceiveVM(const VM& vm) const {
	return free_download_connections_ && CanFit(vm);
}