#ifndef _ES_RGMF_CORE_STATS_H
#define _ES_RGMF_CORE_STATS_H 1

#include <map>
#include <string>

#include "../core/api.h"

namespace fitgalgo
{

template <typename T>
struct DayStats
{
    std::vector<T> stats{};
};

template <typename T>
struct WeekStats
{
    std::map<int, DayStats<T>> days{};
};

template <typename T>
struct MonthStats
{
    std::map<int, WeekStats<T>> weeks{};
};

template <typename T>
struct YearStats
{
    std::map<int, MonthStats<T>> months{};
};

template <typename T>
class Stats
{
private:
    std::map<int, YearStats<T>> years{};

public:
    explicit Stats(const std::map<DateIdx, T>& data);
    const std::map<int, YearStats<T>> get_data() const { return this->years; }
};

} // namespace fitgalgo

#endif // _ES_RGMF_CORE_STATS_H 1
