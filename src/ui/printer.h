#ifndef _ES_RGMF_UI_PRINTER_H
#define _ES_RGMF_UI_PRINTER_H 1

#include <functional>
#include <optional>
#include <iostream>
#include <iomanip>

#include "colors.h"
#include "repr.h"
#include "../utils/string.h"
#include "../core/api.h"
#include "../core/stats.h"

using std::cout;
using std::endl;

namespace fitgalgo
{

template <typename T>
inline void print_optional_stat(
    const std::string& label, const std::optional<T> stat,
    std::function<std::string(const T&)> formatter)
{
    if (stat.has_value())
	cout << std::left << std::setfill('.') << std::setw(40)
	     << label
	     << std::right << std::setfill('.') << std::setw(40)
	     << formatter(stat.value())
	     << endl;
}

inline void print_header(const std::string& header)
{
    cout << colors::GREEN << colors::BOLD << header << endl;
    cout << "--------------------------------------------------------------------------------"
	 << colors::RESET << endl << endl;
}

inline void print_value(const std::string& label, const std::string& value)
{
    cout << std::left << std::setfill('.') << std::setw(40) << label
	 << std::right << std::setfill('.') << std::setw(40) << value
	 << endl;
}

inline std::string value_formatted(
    const std::string& label, const std::string& value, const size_t& w = 40)
{
    size_t label_value_chars = fitgalgo::mb_strlen(label) + fitgalgo::mb_strlen(value);
    size_t fill = w > label_value_chars ? w - label_value_chars : 0;

    std::stringstream ss;
    ss << label;
    for (size_t i = 0; i < fill; i++)
	ss << '.';
    ss << value;
    return ss.str();
}

inline void print_steps_stats(const StepsStats& stats)
{
    const std::unique_ptr<Steps>& steps = stats.get_stats();
    
    print_value("Steps", unit(steps->steps, "steps"));
    print_value("Distance", distance(steps->distance));
    print_value("Calories", calories(steps->calories));
    if (stats.get_count() > 0)
	print_value("Avg. Steps", unit((int) (steps->steps / stats.get_count()), "steps/day"));
    cout << endl;
}

inline void print_sleep_stats(const SleepStats& stats)
{
    const std::unique_ptr<Sleep>& sleep = stats.get_stats();
    const size_t& count = stats.get_count();

    print_value("Sleep Score", unit(sleep->assessment.overall_sleep_score / count));
    print_value("Deep Score", unit(sleep->assessment.deep_sleep_score / count));
    print_value("REM Score", unit(sleep->assessment.rem_sleep_score / count));
    print_value("Light Score", unit(sleep->assessment.light_sleep_score / count));
    print_value("Awekening Count", unit(sleep->assessment.awakenings_count / count));
    print_value("Awekening Count Score", unit(sleep->assessment.awakenings_count_score / count));
    print_value("Duration Score", unit(sleep->assessment.sleep_duration_score / count));
    print_value("Quality Score", unit(sleep->assessment.sleep_quality_score / count));
    print_value("Recovery Score", unit(sleep->assessment.sleep_recovery_score / count));
    print_value("Avg. Stress", unit(sleep->assessment.average_stress_during_sleep / count));
    cout << endl;
}

inline void print_activities_stats(const Activity* a)
{
    cout << colors::BOLD << colors::RED;
    print_optional_stat<float>("Work Time", a->total_work_time, time);
    cout << colors::RESET << colors::RED;
    print_optional_stat<float>("Elapsed Time", a->total_elapsed_time, time);
    print_optional_stat<float>("Timer Time", a->total_timer_time, time);

    cout << colors::RESET << colors::CYAN;
    print_optional_stat<float>("Distance", a->total_distance, elevation);

    cout << colors::RESET << colors::MAGENTA;
    print_optional_stat<float>("Avg Speed", a->avg_speed, elevation);
    print_optional_stat<float>("Max Speed", a->max_speed, elevation);

    cout << colors::RESET << colors::GREEN;
    print_optional_stat<float>("Ascent", a->total_ascent, elevation);
    print_optional_stat<float>("Descent", a->total_descent, elevation);

    cout << colors::RESET;
    print_optional_stat<float>("Total Calories", a->total_calories, calories);
    print_optional_stat<float>("Avg. Temperature", a->avg_temperature, temperature);
    print_optional_stat<float>("Max. Temperature", a->max_temperature, temperature);
    print_optional_stat<float>("Min. Temperature", a->min_temperature, temperature);
    print_optional_stat<float>("Avg. Respiration Rate", a->avg_respiration_rate, value);
    print_optional_stat<float>("Max. Respiration Rate", a->max_respiration_rate, value);
    print_optional_stat<float>("Min. Respiration Rate", a->min_respiration_rate, value);
    print_optional_stat<float>("Training Load Peak", a->training_load_peak, value);
    print_optional_stat<float>("Total Training Effect", a->total_training_effect, value);
    print_optional_stat<float>("Total Anaerobic Training Effect", a->total_anaerobic_training_effect, value);
}

inline void print_sets(const std::vector<Set>& sets)
{
    for (auto const& set : sets)
    {
	print_value("Timestamp", time(set.timestamp));
    }
}

inline void print_sets_activities_stats(const SetsActivity* a)
{
    if (!a->sets.empty())
	print_sets(a->sets);
    print_activities_stats(a);
}

inline void print_splits(const std::vector<Split>& splits)
{
    for (auto const& split : splits)
    {
	print_value("Split Type", split.split_type);
    }
}

inline void print_splits_activities_stats(const SplitsActivity* a)
{
    if (!a->splits.empty())
	print_splits(a->splits);
    print_activities_stats(a);
}

inline void print_aggregated_stats(const AggregatedStats& stats)
{
    const auto& a = stats.get_stats();

    cout << value_formatted("# Activities", std::to_string(stats.get_count()), 40) << endl;
    cout << value_formatted("From date", date(stats.get_from_year_month_day()), 40) << endl;
    cout << value_formatted("To date", date(stats.get_to_year_month_day()), 40) << endl;

    if (a->total_work_time.has_value())
	cout << value_formatted("Work Time", time(a->total_work_time.value()), 40) << endl;
    if (a->total_elapsed_time.has_value())
	cout << value_formatted("Elapsed Time", time(a->total_elapsed_time.value()), 40) << endl;
    if (a->total_timer_time.has_value())
	cout << value_formatted("Timer Time", time(a->total_timer_time.value()), 40) << endl;
    if (a->total_distance.has_value())
	cout << value_formatted("Distance", distance(a->total_distance.value()), 40) << endl;
    if (a->avg_speed.has_value())
	cout << value_formatted("Avg Speed", speed(a->avg_speed.value()), 40) << endl;
    if (a->max_speed.has_value())
	cout << value_formatted("Max Speed", speed(a->max_speed.value()), 40) << endl;
    if (a->total_ascent.has_value())
	cout << value_formatted("Ascent", elevation(a->total_ascent.value()), 40) << endl;
    if (a->total_descent.has_value())
	cout << value_formatted("Descent", elevation(a->total_descent.value()), 40) << endl;
    if (a->total_calories.has_value())
	cout << value_formatted("Calories", calories(a->total_calories.value()), 40) << endl;
    if (a->avg_temperature.has_value())
	cout << value_formatted("Avg Temp", temperature(a->avg_temperature.value()), 40) << endl;
    if (a->max_temperature.has_value())
	cout << value_formatted("Max Temp", temperature(a->max_temperature.value()), 40) << endl;
    if (a->min_temperature.has_value())
	cout << value_formatted("Min Temp", temperature(a->min_temperature.value()), 40) << endl;
    if (a->avg_respiration_rate.has_value())
	cout << value_formatted("Avg Resp", value(a->avg_respiration_rate.value()), 40) << endl;
    if (a->max_respiration_rate.has_value())
	cout << value_formatted("Max Resp", value(a->max_respiration_rate.value()), 40) << endl;
    if (a->min_respiration_rate.has_value())
	cout << value_formatted("Min Resp", value(a->min_respiration_rate.value()), 40) << endl;
    if (a->training_load_peak.has_value())
	cout << value_formatted("Load Peak", value(a->training_load_peak.value()), 40) << endl;
    if (a->total_training_effect.has_value())
	cout << value_formatted("Train Effect", value(a->total_training_effect.value()), 40)
	     << endl;
    if (a->total_anaerobic_training_effect.has_value())
	cout << value_formatted(
	    "Anaerobic Effect", value(a->total_anaerobic_training_effect.value()), 40)
	     << endl;
}

}

#endif // _ES_RGMF_UI_PRINTER_H
