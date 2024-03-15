#ifndef _ES_RGMF_CORE_STATS_H
#define _ES_RGMF_CORE_STATS_H 1

#include <memory>
#include <chrono>

#include "api.h"

namespace fitgalgo
{

class Stats
{
protected:
    std::chrono::year_month_day from;
    std::chrono::year_month_day to;
    size_t count;

public:
    explicit Stats() : from{}, to{}, count{} {}

    bool empty() const;
    size_t get_count() const;
    const std::chrono::year_month_day& get_from_year_month_day() const;
    const std::chrono::year_month_day& get_to_year_month_day() const;
};

class AggregatedStats : public Stats
{
private:
    std::unique_ptr<Activity> activity;

    inline void merge(const std::unique_ptr<Activity>& a);

public:
    explicit AggregatedStats() : activity{} {}
    explicit AggregatedStats(const std::map<DateIdx, std::unique_ptr<Activity>>& activities);
    explicit AggregatedStats(
	const ushort& year, const std::map<DateIdx, std::unique_ptr<Activity>>& activities);
    explicit AggregatedStats(
	const ushort& year, const ushort& month,
	const std::map<DateIdx, std::unique_ptr<Activity>>& activities);

    AggregatedStats(const AggregatedStats& other);
    AggregatedStats& operator=(const AggregatedStats& other);

    const std::unique_ptr<Activity>& get_stats() const;
};

class SportStats
{
private:
    std::map<std::string, AggregatedStats> stats;

public:
    SportStats() = delete;
    explicit SportStats(
	const ushort& year, const std::map<DateIdx, std::unique_ptr<Activity>>& activities);
    explicit SportStats(
	const ushort& year, const ushort& month,
	const std::map<DateIdx, std::unique_ptr<Activity>>& activities);

    bool empty() const;
    const std::map<std::string, AggregatedStats>& get_stats() const;
};

class StepsStats : public Stats
{
private:
    std::unique_ptr<Steps> steps;

public:
    explicit StepsStats();

    const std::unique_ptr<Steps>& get_stats() const;

    StepsStats& operator+=(const Steps& steps);
    StepsStats& operator+=(const StepsStats& other);
    friend StepsStats operator+(StepsStats lhs, const Stats& rhs);
    friend StepsStats operator+(StepsStats lhs, const StepsStats& rhs);
};

class SleepStats : public Stats
{
private:
    std::unique_ptr<Sleep> sleep;

public:
    explicit SleepStats();

    const std::unique_ptr<Sleep>& get_stats() const;

    SleepStats& operator+=(const Sleep& sleep);
    SleepStats& operator+=(const SleepStats& other);
    SleepStats& operator/(int n);
    friend SleepStats operator+(SleepStats lhs, const Stats& rhs);
    friend SleepStats operator+(SleepStats lhs, const SleepStats& rhs);
};

} // namespace fitgalgo

#endif // _ES_RGMF_CORE_STATS_H
