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