#pragma once

#include <set>
#include <stdexcept>
#include <tuple>
#include <vector>

struct VM {
	size_t cpu;
	size_t mem;
	size_t id;
	long double migration_time;
};

struct VMArrangement {
	std::vector<size_t> vm_server; // i-th element is index of i-th VM's server 
};

struct ServerSpec {
	size_t mem;
	size_t cpu;
	size_t max_out;
	size_t max_in;
};

struct Movement {
	size_t from;
	size_t to;
	long double start_moment;
	long double duration;
	size_t vm_id;
};

struct Problem {
	VMArrangement start_position;
	VMArrangement end_position;
	std::vector<VM> vms;
	std::vector<ServerSpec> server_specs;
};

struct Solution {
	std::vector<std::vector<Movement>> vm_movements; // i-th vector - movements of i-th VM

	Solution(size_t vms) : vm_movements(vms) {}
	Solution() = default;
};

class Server {
public:
	Server(const ServerSpec& spec, size_t id);

	void ReceiveVM(const VM& vm);
	void SendVM(const VM& vm);

	void CancelReceivingVM(const VM& vm);
	void CancelSendingVM(const VM& vm);

	bool CanSendVM() const;
	bool CanReceiveVM(const VM& vm) const;

	bool HasVM(size_t vm_id) const;

	std::tuple<size_t, size_t> GetFreeSpace() const; // {cpu, mem}
	bool CanFit(const VM& vm) const;

	std::set<size_t>* GetRawVMSet();

private:
	size_t free_mem_;
	size_t free_cpu_;
	size_t free_download_connections_;
	size_t free_upload_connections_;
	size_t id_;
	std::set<size_t> vms_;
	ServerSpec spec_;
};
