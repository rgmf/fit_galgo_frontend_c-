#include <cstddef>
#include <memory>
#ifndef _ES_RGMF_CORE_STATS_H
#define _ES_RGMF_CORE_STATS_H 1

#include <sys/types.h>
#include <chrono>

#include "api.h"

namespace fitgalgo
{    

class AggregatedStats
{
private:
    std::chrono::year_month_day from;
    std::chrono::year_month_day to;
    size_t count;
    std::unique_ptr<Activity> activity;

    inline void merge(const std::unique_ptr<Activity>& a);

public:
    AggregatedStats() = delete;
    explicit AggregatedStats(const std::map<DateIdx, std::unique_ptr<Activity>>& activities);
    explicit AggregatedStats(
	const ushort& year, const ushort& month,
	const std::map<DateIdx, std::unique_ptr<Activity>>& activities);

    bool empty() const;
    const std::unique_ptr<Activity>& get_aggregated_stats() const;
    const std::chrono::year_month_day& get_from_year_month_day() const;
    const std::chrono::year_month_day& get_to_year_month_day() const;
    size_t get_count() const;
};

} // namespace fitgalgo

#endif // _ES_RGMF_CORE_STATS_H
