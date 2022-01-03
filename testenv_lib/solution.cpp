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

	vms_.insert(vm.id);
}

void Server::SendVM(const VM& vm) {
	if (!free_upload_connections_) {
		throw std::runtime_error("Server #" + std::to_string(id_) + " cannot send so many VMs");
	}

	free_mem_ += vm.mem;
	free_cpu_ += vm.cpu;
	--free_upload_connections_;

	auto it = vms_.find(vm.id);
	if (it == vms_.end()) {
		throw std::runtime_error("No VM#" + std::to_string(vm.id) + " on server");
	}

	vms_.erase(it);
}

void Server::CancelReceivingVM() {
	++free_download_connections_;
}

void Server::CancelSendingVM() {
	++free_upload_connections_;
}

bool Server::HasVM(size_t vm_id) const {
	return (vms_.contains(vm_id));
}

std::tuple<size_t, size_t> Server::GetFreeSpace() const {
	return {free_cpu_, free_mem_};
}