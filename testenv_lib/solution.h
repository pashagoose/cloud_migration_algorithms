#pragma once

#include <set>
#include <stdexcept>
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
};
