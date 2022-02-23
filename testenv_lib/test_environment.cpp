#include "test_environment.h"

#include <stdexcept>
#include <glog/logging.h>

TestEnvironment::TestEnvironment(
	size_t seed,
	size_t diff_percentage_max,
	size_t servers_quantity_min,
	size_t servers_quantity_max
)
	: accumulators_{
		MetricsAccumulator("TotalTime"),
		MetricsAccumulator("TotalMemoryMigration"),
		MetricsAccumulator("SumMigrationTime")
	}
	, generator_(std::make_unique<RealLifeGenerator>(seed, diff_percentage_max, servers_quantity_min, servers_quantity_max))
{
	metrics_.emplace_back(std::make_unique<TotalTime>());
	metrics_.emplace_back(std::make_unique<TotalMemoryMigration>());
	metrics_.emplace_back(std::make_unique<SumMigrationTime>());
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
		servers[problem_.start_position.vm_server[i]].CancelReceivingVM(problem_.vms[i]);
	}

	// Sort all moves

	std::vector<Movement> movements;
	for (const auto& vm_moves : solution_->vm_movements) {
		long double prev_move_time = 0;
		for (const auto& move : vm_moves) {
			if (move.start_moment < prev_move_time) {
				throw std::runtime_error("Moves are intersecting for VM #" + std::to_string(move.vm_id));
			}

			if (move.start_moment < 0) {
				throw std::runtime_error("Move starts at negative timestamp");
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

			servers[passed_move.from].CancelSendingVM(problem_.vms[passed_move.vm_id]);
			servers[passed_move.to].CancelReceivingVM(problem_.vms[passed_move.vm_id]);

			transfer_endings.erase(transfer_endings.begin());
		}

		// Process current transfer
		servers[move.from].SendVM(problem_.vms[move.vm_id]);
		servers[move.to].ReceiveVM(problem_.vms[move.vm_id]);
		transfer_endings.insert({move.start_moment + move.duration, move});
	}

	while (!transfer_endings.empty()) {
		const auto& passed_move = transfer_endings.begin()->second;

		servers[passed_move.from].CancelSendingVM(problem_.vms[passed_move.vm_id]);
		servers[passed_move.to].CancelReceivingVM(problem_.vms[passed_move.vm_id]);

		transfer_endings.erase(transfer_endings.begin());
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

void TestEnvironment::CountMetrics(Metrics::MetricsSet* measurements) {
	Metrics::Metrics* test_measurements = measurements->add_metrics();

	for (size_t i = 0; i < metrics_.size(); ++i) {
		Metrics::Metric* single_measurement = test_measurements->add_measurements();
		single_measurement->set_name(accumulators_[i].GetName());
		long double res = metrics_[i]->Evaluate(problem_, *solution_);
		single_measurement->set_value(res);
		accumulators_[i].AppendMetric(res);
	}
}

void TestEnvironment::PrintMeasurements(std::ostream& out) const {
	for (const auto& accum : accumulators_) {
		out << accum.GetName() << ": " << "mean=" << accum.GetMean() << '\n';
	}
}

void TestEnvironment::ClearMeasurements() {
	for (auto& accum : accumulators_) {
		accum.Clear();
	}
}	

void TestEnvironment::GenerateAndDumpTests(const std::string& path, size_t test_count) {
	std::ofstream file(path, std::ios::binary | std::ios::trunc | std::ios::out);

	if (!file.is_open()) {
		throw std::invalid_argument("Path `" + path + "` seems incorrect for dumping tests");
	}

	DataSet::DataSet dataset;

	for (size_t i = 0; i < test_count; ++i) {
		auto problem = generator_->Generate();

		DataSet::TestCase* test = dataset.add_tests();

		test->set_id(i);

		for (size_t i = 0; i < problem.vms.size(); ++i) {
			DataSet::VM* vm = test->add_vms();

			vm->set_cpu(problem.vms[i].cpu);
			vm->set_mem(problem.vms[i].mem);
			vm->set_id(problem.vms[i].id);
			vm->set_migration_time(problem.vms[i].migration_time);
		}

		for (size_t i = 0; i < problem.server_specs.size(); ++i) {
			DataSet::ServerSpec* spec = test->add_specs();

			spec->set_mem(problem.server_specs[i].mem);
			spec->set_cpu(problem.server_specs[i].cpu);
			spec->set_max_in(problem.server_specs[i].max_in);
			spec->set_max_out(problem.server_specs[i].max_out);
		}

		DataSet::VMArrangement* start_pos = test->mutable_start_position();
		DataSet::VMArrangement* end_pos = test->mutable_end_position();

		for (size_t i = 0; i < problem.vms.size(); ++i) {
			start_pos->add_vm_server(problem.start_position.vm_server[i]); 
			end_pos->add_vm_server(problem.end_position.vm_server[i]);
		}
	}

	dataset.SerializeToOstream(&file);
}

DataSet::DataSet LoadTests(const std::string& path) {
	std::ifstream file(path, std::ios::in | std::ios::binary);

	if (!file.is_open()) {
		throw std::invalid_argument("Cannot load tests from `" + path + "`");
	}

	DataSet::DataSet dataset;
	dataset.ParseFromIstream(&file);

	return dataset;
}

Problem ConvertTestCaseToProblem(const DataSet::TestCase& test) {
	Problem result;

	result.vms.resize(test.vms_size());
	result.server_specs.resize(test.specs_size());
	result.start_position.vm_server.resize(test.start_position().vm_server_size());
	result.end_position.vm_server.resize(test.end_position().vm_server_size());

	for (size_t i = 0; i < test.vms_size(); ++i) {
		result.vms[i].id = test.vms(i).id();
		result.vms[i].mem = test.vms(i).mem();
		result.vms[i].cpu = test.vms(i).cpu();
		result.vms[i].migration_time = test.vms(i).migration_time();
	}

	for (size_t i = 0; i < test.specs_size(); ++i) {
		result.server_specs[i].mem = test.specs(i).mem();
		result.server_specs[i].cpu = test.specs(i).cpu();
		result.server_specs[i].max_in = test.specs(i).max_in();
		result.server_specs[i].max_out = test.specs(i).max_out();
	}

	const DataSet::VMArrangement& start_pos = test.start_position();
	const DataSet::VMArrangement& end_pos = test.end_position();

	for (size_t i = 0; i < start_pos.vm_server_size(); ++i) {
		result.start_position.vm_server[i] = start_pos.vm_server(i);
		result.end_position.vm_server[i] = end_pos.vm_server(i);
	}

	return result;
}