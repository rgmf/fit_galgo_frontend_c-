#include <chrono>
#include <memory>

#include "stats.h"
#include "api.h"
#include "../utils/date.h"

namespace fitgalgo
{

bool Stats::empty() const { return count == 0; }

size_t Stats::get_count() const { return count; }

const std::chrono::year_month_day& Stats::get_from_year_month_day() const
{
    return from;
}
    
const std::chrono::year_month_day& Stats::get_to_year_month_day() const
{
    return to;
}

AggregatedStats::AggregatedStats(const std::map<DateIdx, std::unique_ptr<Activity>>& activities)
{
    count = 0;
    activity = std::make_unique<Activity>();
    from = std::chrono::year_month_day(
	std::chrono::floor<std::chrono::days>(std::chrono::system_clock::now()));
    to = std::chrono::year_month_day();

    for (const auto& [idx, a] : activities)
    {
	merge(a);
	count++;
	if (idx.ymd() < from)
	    from = idx.ymd();
	if (idx.ymd() > to)
	    to = idx.ymd();
    }
}

AggregatedStats::AggregatedStats(
    const ushort& year, const std::map<DateIdx, std::unique_ptr<Activity>>& activities)
{
    count = 0;
    activity = std::make_unique<Activity>();
    from = std::chrono::year_month_day(std::chrono::year(year) / std::chrono::January / 1);
    to = std::chrono::year_month_day(std::chrono::year(year) / std::chrono::December / 31);

    auto itr = std::find_if(
	activities.cbegin(),
	activities.cend(),
	[year](const auto& item) { return item.first.year() == year; });
    if (itr == activities.end())
	return;

    while (itr != activities.end() && itr->first.year() == year)
    {
	merge(itr->second);
	count++;
	itr++;
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

    while (itr != activities.end() && itr->first.year() == year && itr->first.month() == month)
    {
	merge(itr->second);
	count++;
	itr++;
    }
}

AggregatedStats::AggregatedStats(const AggregatedStats& other)
{
    from = other.from;
    to = other.to;
    count = other.count;
    if (other.activity)
    {
	if (auto distance_a = dynamic_cast<const DistanceActivity*>(other.activity.get()))
	    activity = std::make_unique<DistanceActivity>(*distance_a);
	else if (auto sets_a = dynamic_cast<const SetsActivity*>(other.activity.get()))
	    activity = std::make_unique<SetsActivity>(*sets_a);
	else if (auto splits_a = dynamic_cast<const SplitsActivity*>(other.activity.get()))
	    activity = std::make_unique<SplitsActivity>(*splits_a);
	else
	    activity = std::make_unique<Activity>(*other.activity);
    }
}

AggregatedStats& AggregatedStats::operator=(const AggregatedStats& other)
{
    if (this != &other)
    {
	from = other.from;
	to = other.to;
	count = other.count;

	if (other.activity)
	{
	    if (auto distance_a = dynamic_cast<const DistanceActivity*>(other.activity.get()))
		activity = std::make_unique<DistanceActivity>(*distance_a);
	    else if (auto sets_a = dynamic_cast<const SetsActivity*>(other.activity.get()))
		activity = std::make_unique<SetsActivity>(*sets_a);
	    else if (auto splits_a = dynamic_cast<const SplitsActivity*>(other.activity.get()))
		activity = std::make_unique<SplitsActivity>(*splits_a);
	    else
		activity = std::make_unique<Activity>(*other.activity);
	}
	else
	    activity.reset();
    }

    return *this;
}

const std::unique_ptr<Activity>& AggregatedStats::get_stats() const
{
    return activity;
}

std::optional<float> acc_optional(const std::optional<float> &lhs,
                                  const std::optional<float> &rhs)
{
    return lhs.has_value() ?
	(rhs.has_value() ? std::optional<float>(lhs.value() + rhs.value()) : lhs) :
	rhs;
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

SportStats::SportStats(
    const ushort& year, const std::map<DateIdx, std::unique_ptr<Activity>>& activities)
{
    stats = {};

    auto itr = std::find_if(
	activities.cbegin(),
	activities.cend(),
	[year](const auto& item) { return item.first.year() == year; });
    if (itr == activities.end())
	return;

    auto activities_by_sport = std::map<std::string, std::map<DateIdx, std::unique_ptr<Activity>>>();
    while (itr != activities.end() && itr->first.year() == year)
    {
	std::unique_ptr<Activity> new_activity;

        switch (itr->second->get_id())
	{
	case ActivityType::DISTANCE:
	    new_activity = std::make_unique<DistanceActivity>(
		dynamic_cast<DistanceActivity&>(*itr->second));
	    break;
	case ActivityType::SETS:
	    new_activity = std::make_unique<SetsActivity>(
		dynamic_cast<SetsActivity&>(*itr->second));
	    break;
	case ActivityType::SPLITS:
	    new_activity = std::make_unique<SplitsActivity>(
		dynamic_cast<SplitsActivity&>(*itr->second));
	    break;
	default:
	    new_activity = std::make_unique<Activity>(*itr->second);
	    break;
	}

        activities_by_sport[itr->second->sport][itr->first] = std::move(new_activity);

	itr++;
    }

    for (const auto& [sport, a_map] : activities_by_sport)
    {
	stats[sport] = AggregatedStats(a_map);
    }
}

SportStats::SportStats(
    const ushort& year, const ushort& month,
    const std::map<DateIdx, std::unique_ptr<Activity>>& activities)
{
    stats = {};

    auto itr = std::find_if(
	activities.cbegin(),
	activities.cend(),
	[year, month](const auto& item) {
	    return item.first.year() == year && item.first.month() == month;
	});
    if (itr == activities.end())
	return;

    auto activities_by_sport = std::map<std::string, std::map<DateIdx, std::unique_ptr<Activity>>>();
    while (itr != activities.end() && itr->first.year() == year && itr->first.month() == month)
    {
	std::unique_ptr<Activity> new_activity = std::make_unique<Activity>(*itr->second);
	activities_by_sport[itr->second->sport][itr->first] = std::move(new_activity);
	
	itr++;
    }

    for (const auto& [sport, a_map] : activities_by_sport)
    {
	stats[sport] = AggregatedStats(a_map);
    }
}

bool SportStats::empty() const
{
    return stats.empty();
}

const std::map<std::string, AggregatedStats>& SportStats::get_stats() const
{
    return stats;
}

StepsStats::StepsStats()
{
    steps = std::make_unique<Steps>();
}

StepsStats& StepsStats::operator+=(const Steps &rhs)
{
    count++;

    if (steps->datetime_utc.empty())
	steps->datetime_utc = rhs.datetime_utc;
    if (steps->datetime_local.empty())
	steps->datetime_local = rhs.datetime_local;

    if (from.year() != std::chrono::year{0} &&
	from.month() != std::chrono::month{0} &&
	from.day() != std::chrono::day{0})
    {
	from = from_isodate_to_ymd(rhs.datetime_utc);
    }

    to = from_isodate_to_ymd(rhs.datetime_utc);

    steps->steps += rhs.steps;
    steps->distance += rhs.steps;
    steps->calories += rhs.steps;

    return *this;
}

StepsStats &StepsStats::operator+=(const StepsStats &rhs)
{
    count++;

    if (steps->datetime_utc.empty())
	steps->datetime_utc = rhs.steps->datetime_utc;
    if (steps->datetime_local.empty())
	steps->datetime_local = rhs.steps->datetime_local;

    if (from.year() != std::chrono::year{0} &&
	from.month() != std::chrono::month{0} &&
	from.day() != std::chrono::day{0})
    {
	from = rhs.from;
    }

    to = rhs.to;

    steps->steps += rhs.steps->steps;
    steps->distance += rhs.steps->distance;
    steps->calories += rhs.steps->calories;

    return *this;
}

StepsStats operator+(StepsStats lhs, const Steps& rhs)
{
    lhs += rhs;
    return lhs;
}

StepsStats operator+(StepsStats lhs, const StepsStats& rhs)
{
    lhs += rhs;
    return lhs;
}

const std::unique_ptr<Steps> &StepsStats::get_stats() const
{
    return steps;
}

SleepStats::SleepStats()
{
    sleep = std::make_unique<Sleep>();
}

SleepStats& SleepStats::operator+=(const Sleep &rhs)
{
    count++;

    if (sleep->zone_info.empty())
	sleep->zone_info = rhs.zone_info;

    sleep->assessment.combined_awake_score += rhs.assessment.combined_awake_score;
    sleep->assessment.awake_time_score += rhs.assessment.awake_time_score;	
    sleep->assessment.awakenings_count_score += rhs.assessment.awakenings_count_score;	
    sleep->assessment.deep_sleep_score += rhs.assessment.deep_sleep_score;	
    sleep->assessment.sleep_duration_score += rhs.assessment.sleep_duration_score;	
    sleep->assessment.light_sleep_score += rhs.assessment.light_sleep_score;	
    sleep->assessment.overall_sleep_score += rhs.assessment.overall_sleep_score;	
    sleep->assessment.sleep_quality_score += rhs.assessment.sleep_quality_score;	
    sleep->assessment.sleep_recovery_score += rhs.assessment.sleep_recovery_score;	
    sleep->assessment.rem_sleep_score += rhs.assessment.rem_sleep_score;	
    sleep->assessment.sleep_restlessness_score += rhs.assessment.sleep_restlessness_score;
    sleep->assessment.awakenings_count += rhs.assessment.awakenings_count;
    sleep->assessment.interruptions_score += rhs.assessment.interruptions_score;	
    sleep->assessment.average_stress_during_sleep += rhs.assessment.average_stress_during_sleep;
    
    sleep->levels.insert(sleep->levels.end(), rhs.levels.begin(), rhs.levels.end());
    sleep->dates.insert(sleep->dates.end(), rhs.dates.begin(), rhs.dates.end());

    return *this;
}

SleepStats &SleepStats::operator+=(const SleepStats &rhs)
{
    count++;

    if (sleep->zone_info.empty())
	sleep->zone_info = rhs.sleep->zone_info;

    sleep->assessment.combined_awake_score += rhs.sleep->assessment.combined_awake_score;
    sleep->assessment.awake_time_score += rhs.sleep->assessment.awake_time_score;	
    sleep->assessment.awakenings_count_score += rhs.sleep->assessment.awakenings_count_score;	
    sleep->assessment.deep_sleep_score += rhs.sleep->assessment.deep_sleep_score;	
    sleep->assessment.sleep_duration_score += rhs.sleep->assessment.sleep_duration_score;	
    sleep->assessment.light_sleep_score += rhs.sleep->assessment.light_sleep_score;	
    sleep->assessment.overall_sleep_score += rhs.sleep->assessment.overall_sleep_score;	
    sleep->assessment.sleep_quality_score += rhs.sleep->assessment.sleep_quality_score;	
    sleep->assessment.sleep_recovery_score += rhs.sleep->assessment.sleep_recovery_score;	
    sleep->assessment.rem_sleep_score += rhs.sleep->assessment.rem_sleep_score;	
    sleep->assessment.sleep_restlessness_score += rhs.sleep->assessment.sleep_restlessness_score;
    sleep->assessment.awakenings_count += rhs.sleep->assessment.awakenings_count;
    sleep->assessment.interruptions_score += rhs.sleep->assessment.interruptions_score;	
    sleep->assessment.average_stress_during_sleep += rhs.sleep->assessment.average_stress_during_sleep;

    sleep->levels.insert(sleep->levels.end(), rhs.sleep->levels.begin(), rhs.sleep->levels.end());
    sleep->dates.insert(sleep->dates.end(), rhs.sleep->dates.begin(), rhs.sleep->dates.end());

    return *this;
}

SleepStats operator+(SleepStats lhs, const Sleep& rhs)
{
    lhs += rhs;
    return lhs;
}

SleepStats operator+(SleepStats lhs, const SleepStats& rhs)
{
    lhs += rhs;
    return lhs;
}

SleepStats& SleepStats::operator/(int n)
{
    sleep->assessment.combined_awake_score /= n;
    sleep->assessment.awake_time_score /= n;
    sleep->assessment.awakenings_count_score /= n;
    sleep->assessment.deep_sleep_score /= n;
    sleep->assessment.sleep_duration_score /= n;
    sleep->assessment.light_sleep_score /= n;
    sleep->assessment.overall_sleep_score /= n;
    sleep->assessment.sleep_quality_score /= n;
    sleep->assessment.sleep_recovery_score /= n;
    sleep->assessment.rem_sleep_score /= n;
    sleep->assessment.sleep_restlessness_score /= n;
    sleep->assessment.awakenings_count /= n;
    sleep->assessment.interruptions_score /= n;
    sleep->assessment.average_stress_during_sleep /= n;
    return *this;
}

const std::unique_ptr<Sleep>& SleepStats::get_stats() const
{
    return sleep;
}

}
