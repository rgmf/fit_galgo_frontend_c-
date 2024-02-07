#include "stats.h"
#include "api.h"
#include <chrono>
#include <memory>

namespace fitgalgo
{

AggregatedStats::AggregatedStats(const std::map<DateIdx, std::unique_ptr<Activity>>& activities)
{
    count = 0;
    activity = std::make_unique<Activity>();
    from = std::chrono::year_month_day(
	std::chrono::floor<std::chrono::days>(std::chrono::system_clock::now()));
    to = std::chrono::year_month_day();

    for (const auto& [idx, activty] : activities)
    {
	merge(activty);
	count++;
	if (idx.ymd() < from)
	    from = idx.ymd();
	if (idx.ymd() > to)
	    to = idx.ymd();
    }
}

AggregatedStats::AggregatedStats(
    const ushort& year, const ushort& month,
    const std::map<DateIdx, std::unique_ptr<Activity>>& activities)
{
    from = std::chrono::year_month_day(
	std::chrono::year(year), std::chrono::month(month), std::chrono::day(1));
    to = std::chrono::year_month_day_last(from.year(), from.month() / std::chrono::last);
    count = 0;
    activity = std::make_unique<Activity>();

    auto itr = std::find_if(
	activities.cbegin(),
	activities.cend(),
	[year, month](const auto& item) {
	    return item.first.year() == year && item.first.month() == month;
	});
    if (itr == activities.end())
	return;

    while (itr != activities.end() && (itr->first.year() == year && itr->first.month() == month))
    {
	merge(itr->second);
	count++;
	itr++;
    }
}

bool AggregatedStats::empty() const
{
    return count == 0;
}
    
const std::unique_ptr<Activity>& AggregatedStats::get_aggregated_stats() const
{
    return activity;
}

const std::chrono::year_month_day& AggregatedStats::get_from_year_month_day() const
{
    return from;
}
    
const std::chrono::year_month_day& AggregatedStats::get_to_year_month_day() const
{
    return to;
}

size_t AggregatedStats::get_count() const
{
    return count;
}

std::optional<float> acc_optional(const std::optional<float> &lhs,
                                  const std::optional<float> &rhs)
{
    return lhs.has_value() ? (rhs.has_value() ? std::optional<float>(lhs.value() + rhs.value()) : lhs) : rhs;
}

std::optional<float> avg_optional(const std::optional<float> &lhs,
                                  const std::optional<float> &rhs)
{
    if (!lhs.has_value() || lhs.value() == 0)
	return rhs;
    if (!rhs.has_value() || rhs.value() == 0)
	return lhs;
    return std::optional<float>((lhs.value() + rhs.value()) / 2);
}

std::optional<float> max_optional(const std::optional<float> &lhs,
                                  const std::optional<float> &rhs)
{
    if (!lhs.has_value())
	return rhs;
    if (!rhs.has_value())
	return lhs;
    return std::optional<float>(lhs.value() > rhs.value() ? lhs.value() : rhs.value());
}

std::optional<float> min_optional(const std::optional<float> &lhs,
                                  const std::optional<float> &rhs)
{
    if (!lhs.has_value())
	return rhs;
    if (!rhs.has_value())
	return lhs;
    return std::optional<float>(lhs.value() < rhs.value() ? lhs.value() : rhs.value());
}

inline void AggregatedStats::merge(const std::unique_ptr<Activity>& a)
{
    if (activity->zone_info.empty())
	activity->zone_info = a->zone_info;
    if (activity->username.empty())
	activity->username = a->username;
    if (activity->sport_profile_name.empty())
	activity->sport_profile_name = a->sport_profile_name;
    if (activity->sport.empty())
	activity->sport = a->sport;
    if (activity->sub_sport.empty())
	activity->sub_sport = a->sub_sport;
    if (!activity->start_lat_lon.has_value())
	activity->start_lat_lon = a->start_lat_lon;
    if (a->end_lat_lon.has_value())
	activity->end_lat_lon = a->end_lat_lon;
    if (activity->start_time_utc.empty())
	activity->start_time_utc = a->start_time_utc;
    activity->total_elapsed_time = acc_optional(activity->total_elapsed_time, a->total_elapsed_time);
    activity->total_timer_time = acc_optional(activity->total_timer_time, a->total_timer_time);
    activity->total_work_time = acc_optional(activity->total_work_time, a->total_work_time);
    activity->total_distance = acc_optional(activity->total_distance, a->total_distance);
    activity->avg_speed = avg_optional(activity->avg_speed, a->avg_speed);
    activity->max_speed = max_optional(activity->max_speed, a->max_speed);
    activity->avg_cadence = avg_optional(activity->avg_cadence, a->avg_cadence);
    activity->max_cadence = max_optional(activity->max_cadence, a->max_cadence);
    activity->avg_running_cadence = avg_optional(
	activity->avg_running_cadence, a->avg_running_cadence);
    activity->max_running_cadence = max_optional(
	activity->max_running_cadence, a->max_running_cadence);
    activity->total_strides = acc_optional(activity->total_strides, a->total_strides);
    activity->total_calories = acc_optional(activity->total_calories, a->total_calories);
    activity->total_ascent = acc_optional(activity->total_ascent, a->total_ascent);
    activity->total_descent = acc_optional(activity->total_descent, a->total_descent);
    activity->avg_temperature = avg_optional(activity->avg_temperature, a->avg_temperature);
    activity->max_temperature = max_optional(activity->max_temperature, a->max_temperature);
    activity->min_temperature = min_optional(activity->min_temperature, a->min_temperature);
    activity->avg_respiration_rate = avg_optional(
	activity->avg_respiration_rate, a->avg_respiration_rate);
    activity->max_respiration_rate = max_optional(
	activity->max_respiration_rate, a->max_respiration_rate);
    activity->min_respiration_rate = min_optional(
	activity->min_respiration_rate, a->min_respiration_rate);
    activity->training_load_peak = acc_optional(
	activity->training_load_peak, a->training_load_peak);
    activity->total_training_effect = acc_optional(
	activity->total_training_effect, a->total_training_effect);
    activity->total_anaerobic_training_effect = acc_optional(
	activity->total_anaerobic_training_effect, a->total_anaerobic_training_effect);
}

}
