#pragma once

#include "solution.h"

class IMetric {
public:
	virtual long double Evaluate(const Problem& task, const Solution& solution) = 0;
};


class TotalTime final : public IMetric {
public:
	long double Evaluate(const Problem& task, const Solution& solution) override;
};


class SumMigrationTime final : public IMetric {
public:
	long double Evaluate(const Problem& task, const Solution& solution) override;
};


class TotalMemoryMigration final : public IMetric {
public:
	long double Evaluate(const Problem& task, const Solution& solution) override;
};

class TotalSteps final : public IMetric {
public:
	long double Evaluate(const Problem& task, const Solution& solution) override;
};

class MetricsAccumulator {
public:
	MetricsAccumulator(std::string_view name);

	void Clear();
	void AppendMetric(long double val);

	long double GetMean() const;
	std::string GetName() const;

	size_t TotalCount() const;

private:
	std::string metric_name_;
	std::vector<long double> measurements_;
};