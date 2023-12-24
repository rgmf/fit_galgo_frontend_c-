#include <algorithm>
#include <chrono>
#include <cstddef>
#include <format>
#include <numeric>
#include <string>
#include <iostream>
#include <sstream>

#include "shell.h"
#include "calendar.h"
#include "../core/stats.h"
#include "../core/api.h"

using std::cin;
using std::cout;
using std::endl;

namespace fitgalgo
{

inline ushort ask_for_year()
{
    ushort year;
    do
    {
	cout << "Type a year: ";
	cin >> year;
    } while (year < 1000 || year > 9999);
    cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    return year;
}

inline ushort ask_for_month()
{
    ushort month;
    do
    {
	cout << "Type a month (1-12): ";
	cin >> month;
    } while (month < 1 || month > 12);
    cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    return month;
}

template <typename T>
std::string formatted_number(const T& n)
{
    std::string res = std::to_string(n);
    size_t pos = res.find('.');
    short mils_pos{};

    if (pos == std::string::npos)
	mils_pos = res.length() > 3 ? res.length() - 3: 0;
    else
	mils_pos -= 3;
    
    while (mils_pos > 0)
    {
        res.insert(mils_pos, " ");
        mils_pos = mils_pos > 3 ? mils_pos - 3 : 0;
    }

    size_t decimal_pos = res.find('.');
    if (decimal_pos != std::string::npos && decimal_pos + 3 < res.length())
	res = res.substr(0, decimal_pos + 3);

    return res;
}

inline void print_steps_stats(const std::string& h, const Steps& s)
{
    cout << h << endl;
    cout << "-----------------------------------------------------" << endl;
    cout << std::left << std::setfill('.') << std::setw(20) << "Steps"
	 << std::right << std::setfill('.') << std::setw(20)
	 << formatted_number(s.steps) << " steps" << endl;
    cout << std::left << std::setfill('.') << std::setw(20) << "Distance"
	 << std::right << std::setfill('.') << std::setw(20)
	 << formatted_number(static_cast<int>(std::round(s.distance))) << " m" << endl;
    cout << std::left << std::setfill('.') << std::setw(20) << "Calories"
	 << std::right << std::setfill('.') << std::setw(20)
	 << formatted_number(s.calories) << " kcal" << endl << endl;
}

void ShellSteps::loop() const
{
    ushort option;

    do {
	system("clear");
	cout << "STEPS STATS" << endl;
	cout << "-------------------------------------------" << endl;
	cout << "1.- Yearly stats" << endl;
	cout << "2.- Stats for a year" << endl;
	cout << "3.- Stats for a year/month" << endl;
	cout << "4.- Stats for a year/month/week" << endl;
	cout << "0.- Exit" << endl;
	cout << "Select an option: ";
	cin >> option;
	cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

	switch (option)
	{
	case 1:
	    this->year_stats();
	    cout << endl << "Press Enter to continue...";
	    cin.get();
	    break;
	case 2:
	    this->month_stats(ask_for_year());
	    cout << endl << "Press Enter to continue...";
	    cin.get();
	    break;
	case 3:
	    this->week_stats(ask_for_year(), ask_for_month());
	    cout << endl << endl << "Press Enter to continue...";
	    cin.get();
	    break;
	}
    } while (option != 0);
}

void ShellSteps::year_stats() const
{
    system("clear");
    cout << "YEARLY STEPS STATS" << endl;
    cout << "=====================================================" << endl;
    for (const auto& [year, years] : this->stats.get_data())
    {
        Steps all_steps = std::accumulate(
            years.months.begin(), years.months.end(), Steps{},
            [](const auto& acc, const auto& month_entry) {
                return std::accumulate(
                    month_entry.second.weeks.begin(), month_entry.second.weeks.end(), acc,
                    [](const auto& acc, const auto& week_entry) {
                        return std::accumulate(
                            week_entry.second.days.begin(), week_entry.second.days.end(), acc,
                            [](const auto& acc, const auto& day_entry) {
				return std::accumulate(
				    day_entry.second.stats.begin(), day_entry.second.stats.end(), acc,
				    [](const auto& acc, const auto& object) {
					return acc + object;
				    });
                            });
                    });
            });
	print_steps_stats(std::format("Year {}", year), all_steps);
    }
}

void ShellSteps::month_stats(const ushort& year) const
{
    system("clear");
    cout << "MONTHLY STEPS STATS FOR YEAR " << year << endl;
    cout << "=====================================================" << endl;
    const auto& data = this->stats.get_data();
    if (!data.contains(year))
    {
	cout << "There are not data for this year" << endl;
	return;
    }

    for (const auto& [month, months] : data.at(year).months)
    {
	Steps all_steps = std::accumulate(
	    months.weeks.begin(), months.weeks.end(), Steps{},
	    [](const auto& acc, const auto& week_entry) {
		return std::accumulate(
		    week_entry.second.days.begin(), week_entry.second.days.end(), acc,
		    [](const auto& acc, const auto& day_entry) {
			return std::accumulate(
			    day_entry.second.stats.begin(), day_entry.second.stats.end(), acc,
			    [](const auto& acc, const auto& object) {
				return acc + object;
			    });
		    });
	    });
	print_steps_stats(MONTHS_NAMES[month], all_steps);
    }
}

void ShellSteps::week_stats(const ushort& year, const ushort& month) const
{
    system("clear");
    cout << MONTHS_NAMES[month] << ", " << year << endl;
    cout << "=====================================================" << endl;

    const auto& years = this->stats.get_data();
    if (!years.contains(year))
    {
	cout << "There are not data for year " << year << endl;
	return;
    }

    const auto& ystats = years.at(year);
    if (!ystats.months.contains(month - 1))
    {
	cout << "There are not data for month " << MONTHS_NAMES[month] << endl;
	return;
    }

    const auto& mstats = ystats.months.at(month - 1);
    MonthStats<Steps> month_stats(mstats);

    // Complete first week with days off this month.
    if (ystats.months.contains(month - 2))
    {
	const auto& mstats_before = ystats.months.at(month - 2);
	for (auto itr = mstats_before.weeks.begin(); itr != mstats_before.weeks.end(); itr++)
	{
	    if (mstats.weeks.contains(itr->first))
	    {
		for (auto iday = itr->second.days.begin(); iday != itr->second.days.end(); iday++)
		{
		    month_stats.weeks[itr->first].days[iday->first] = iday->second;
		}
	    }
	}
    }

    // Complete last week with days off this month.
    if (ystats.months.contains(month))
    {
	const auto& mstats_after = ystats.months.at(month);
	for (auto itr = mstats_after.weeks.begin(); itr != mstats_after.weeks.end(); itr++)
	{
	    if (mstats.weeks.contains(itr->first))
	    {
		// cout << "ESTÁ: " << itr->first << endl;
		for (auto iday = itr->second.days.begin(); iday != itr->second.days.end(); iday++)
		{
		    month_stats.weeks[itr->first].days[iday->first] = iday->second;
		}
	    }
	}
    }

    std::map<std::chrono::year_month_day, Steps> steps_by_day{};
    for (auto itr = month_stats.weeks.begin(); itr != month_stats.weeks.end(); itr++)
    {
	for (auto itr_day = itr->second.days.begin();
	     itr_day != itr->second.days.end();
	     itr_day++)
	{
	    for (const auto& steps : itr_day->second.stats)
	    {
		std::istringstream stream(steps.datetime_local);
		int year, month, day;
		char dash;

		stream >> year >> dash >> month >> dash >> day;
		if (!stream.fail() && !stream.bad())
		{
		    std::chrono::year_month_day ymd{
			std::chrono::year(year),
			std::chrono::month(month),
			std::chrono::day(day)};
		    steps_by_day[ymd] += steps;
		}
	    }
	}
    }

    Calendar calendar(year, month);
    auto ymd = calendar.get_first_wd_ymd();
    auto last_ymd = calendar.get_last_wd_ymd();
    while (ymd <= last_ymd)
    {
	if (steps_by_day.contains(ymd))
	{
	    std::string s1 = std::to_string(steps_by_day[ymd].steps) + " steps";
	    std::string s2 = std::to_string(
		static_cast<int>(std::round(steps_by_day[ymd].distance))) + " m";
	    std::string s3 = std::to_string(steps_by_day[ymd].calories) + " kcal";

	    calendar.add(ymd, s1);
	    calendar.add(ymd, s2);
	    calendar.add(ymd, s3);
	}

	auto sys_days = std::chrono::sys_days(ymd);
	ymd = std::chrono::year_month_day(sys_days + std::chrono::days(1));
    }

    calendar.print();
}

inline void print_sleep_stats(const std::string& h, const Sleep& s, const size_t& count)
{
    cout << h << endl;
    cout << "-----------------------------------------------------" << endl;
    cout << std::left << std::setfill('.') << std::setw(20) << "Sleep Score"
	 << std::right << std::setfill('.') << std::setw(20)
	 << formatted_number(s.assessment.overall_sleep_score / count) << endl
	 << std::left << std::setfill('.') << std::setw(20) << "Deep Score"
	 << std::right << std::setfill('.') << std::setw(20)
	 << formatted_number(s.assessment.deep_sleep_score / count) << endl
	 << std::left << std::setfill('.') << std::setw(20) << "REM Score"
	 << std::right << std::setfill('.') << std::setw(20)
	 << formatted_number(s.assessment.rem_sleep_score / count) << endl
	 << std::left << std::setfill('.') << std::setw(20) << "Light Score"
	 << std::right << std::setfill('.') << std::setw(20)
	 << formatted_number(s.assessment.light_sleep_score / count) << endl
	 << std::left << std::setfill('.') << std::setw(20) << "Awekening Count"
	 << std::right << std::setfill('.') << std::setw(20)
	 << formatted_number(s.assessment.awakenings_count / count) << endl
	 << std::left << std::setfill('.') << std::setw(20) << "Duration Score"
	 << std::right << std::setfill('.') << std::setw(20)
	 << formatted_number(s.assessment.sleep_duration_score / count) << endl
	 << std::left << std::setfill('.') << std::setw(20) << "Quality Score"
	 << std::right << std::setfill('.') << std::setw(20)
	 << formatted_number(s.assessment.sleep_quality_score / count) << endl
	 << std::left << std::setfill('.') << std::setw(20) << "Recovery Score"
	 << std::right << std::setfill('.') << std::setw(20)
	 << formatted_number(s.assessment.sleep_recovery_score / count) << endl
	 << std::left << std::setfill('.') << std::setw(20) << "Avg. Stress"
	 << std::right << std::setfill('.') << std::setw(20)
	 << formatted_number(s.assessment.average_stress_during_sleep / count) << endl << endl;
}

void ShellSleep::loop() const
{
    ushort option;

    do {
	system("clear");
	cout << "SLEEP STATS" << endl;
	cout << "-------------------------------------------" << endl;
	cout << "1.- Yearly stats" << endl;
	cout << "2.- Stats for a year" << endl;
	cout << "3.- Stats for a year/month" << endl;
	cout << "4.- Stats for a year/month/week" << endl;
	cout << "0.- Exit" << endl;
	cout << "Select an option: ";
	cin >> option;
	cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

	switch (option)
	{
	case 1:
	    this->year_stats();
	    cout << endl << "Press Enter to continue...";
	    cin.get();
	    break;
	case 2:
	    this->month_stats(ask_for_year());
	    cout << endl << "Press Enter to continue...";
	    cin.get();
	    break;
	case 3:
	    this->week_stats(ask_for_year(), ask_for_month());
	    cout << endl << endl << "Press Enter to continue...";
	    cin.get();
	    break;
	}
    } while (option != 0);
}

void ShellSleep::year_stats() const
{
    system("clear");
    cout << "YEARLY SLEEP STATS" << endl;
    cout << "=====================================================" << endl;
    for (const auto& [year, years] : this->stats.get_data())
    {
	SleepWithCount sleep_with_count = std::accumulate(
            years.months.begin(), years.months.end(), SleepWithCount{},
            [](const auto& acc, const auto& month_entry) {
                return std::accumulate(
                    month_entry.second.weeks.begin(), month_entry.second.weeks.end(), acc,
                    [](const auto& acc, const auto& week_entry) {
                        return std::accumulate(
                            week_entry.second.days.begin(), week_entry.second.days.end(), acc,
                            [](const auto& acc, const auto& day_entry) {
				return std::accumulate(
				    day_entry.second.stats.begin(), day_entry.second.stats.end(), acc,
				    [](const auto& acc, const auto& object) {
					return SleepWithCount{acc.sleep + object, acc.count + 1};
				    });
                            });
                    });
            });
	print_sleep_stats(std::format("Year {}", year), sleep_with_count.sleep, sleep_with_count.count);
    }
}

void ShellSleep::month_stats(const ushort& year) const
{
    system("clear");
    cout << "MONTHLY SLEEP STATS FOR YEAR " << year << endl;
    cout << "=====================================================" << endl;
    const auto& data = this->stats.get_data();
    if (!data.contains(year))
    {
	cout << "There are not data for this year" << endl;
	return;
    }

    for (const auto& [month, months] : data.at(year).months)
    {
	SleepWithCount sleep_with_count = std::accumulate(
	    months.weeks.begin(), months.weeks.end(), SleepWithCount{},
	    [](const auto& acc, const auto& week_entry) {
		return std::accumulate(
		    week_entry.second.days.begin(), week_entry.second.days.end(), acc,
		    [](const auto& acc, const auto& day_entry) {
			return std::accumulate(
			    day_entry.second.stats.begin(), day_entry.second.stats.end(), acc,
			    [](const auto& acc, const auto& object) {
				return SleepWithCount{acc.sleep + object, acc.count + 1};
			    });
		    });
	    });
	print_sleep_stats(MONTHS_NAMES[month], sleep_with_count.sleep, sleep_with_count.count);
    }
}

void ShellSleep::week_stats(const ushort& year, const ushort& month) const
{
    system("clear");
    cout << MONTHS_NAMES[month] << ", " << year << endl;
    cout << "=====================================================" << endl;

    const auto& years = this->stats.get_data();
    if (!years.contains(year))
    {
	cout << "There are not data for year " << year << endl;
	return;
    }

    const auto& ystats = years.at(year);
    if (!ystats.months.contains(month - 1))
    {
	cout << "There are not data for month " << MONTHS_NAMES[month] << endl;
	return;
    }

    const auto& mstats = ystats.months.at(month - 1);
    MonthStats<Sleep> month_stats(mstats);

    // Complete first week with days off this month.
    if (ystats.months.contains(month - 2))
    {
	const auto& mstats_before = ystats.months.at(month - 2);
	for (auto itr = mstats_before.weeks.begin(); itr != mstats_before.weeks.end(); itr++)
	{
	    if (mstats.weeks.contains(itr->first))
	    {
		for (auto iday = itr->second.days.begin(); iday != itr->second.days.end(); iday++)
		{
		    month_stats.weeks[itr->first].days[iday->first] = iday->second;
		}
	    }
	}
    }

    // Complete last week with days off this month.
    if (ystats.months.contains(month))
    {
	const auto& mstats_after = ystats.months.at(month);
	for (auto itr = mstats_after.weeks.begin(); itr != mstats_after.weeks.end(); itr++)
	{
	    if (mstats.weeks.contains(itr->first))
	    {
		for (auto iday = itr->second.days.begin(); iday != itr->second.days.end(); iday++)
		{
		    month_stats.weeks[itr->first].days[iday->first] = iday->second;
		}
	    }
	}
    }

    std::map<std::chrono::year_month_day, Sleep> sleep_by_day{};
    for (auto itr = month_stats.weeks.begin(); itr != month_stats.weeks.end(); itr++)
    {
	for (auto itr_day = itr->second.days.begin();
	     itr_day != itr->second.days.end();
	     itr_day++)
	{
	    for (const auto& sleep : itr_day->second.stats)
	    {
		if (sleep.dates.size() == 2)
		{
		    std::string dtl{sleep.dates[0]};

		    if (sleep.is_early_morning())
		    {
			// From string to year_month_day
			std::istringstream ss(sleep.dates[0].substr(0, 10));
			int year, month, day;
			char dash1, dash2;
			ss >> year >> dash1 >> month >> dash2 >> day;
			auto ymd = std::chrono::year(year) /
			    std::chrono::month(month) /
			    std::chrono::day(day);

			// Substract one day to ymd
			auto yesterday = std::chrono::year_month_day(
			    std::chrono::sys_days(ymd) - std::chrono::days(1));

			// From year_month_day to string
			std::ostringstream oss;
			oss << std::setw(4) << std::setfill('0')
			    << static_cast<int>(yesterday.year())
			    << '-' << std::setw(2) << std::setfill('0')
			    << static_cast<unsigned>(yesterday.month())
			    << '-' << std::setw(2) << std::setfill('0')
			    << static_cast<unsigned>(yesterday.day());

			// Update dtl
			dtl = oss.str();
		    }

		    std::istringstream stream(dtl);
		    int year, month, day;
		    char dash;

		    stream >> year >> dash >> month >> dash >> day;
		    if (!stream.fail() && !stream.bad())
		    {
			std::chrono::year_month_day ymd{
			    std::chrono::year(year),
			    std::chrono::month(month),
			    std::chrono::day(day)};
			sleep_by_day[ymd] = sleep_by_day.contains(ymd) ?
			    (sleep_by_day[ymd] + sleep) / 2 :
			    sleep;
		    }
		}
	    }
	}
    }

    Calendar calendar(year, month);
    auto ymd = calendar.get_first_wd_ymd();
    auto last_ymd = calendar.get_last_wd_ymd();
    while (ymd <= last_ymd)
    {
	if (sleep_by_day.contains(ymd))
	{
	    std::string s1 = "Overall    " +
		std::to_string(
		    static_cast<int>(std::round(sleep_by_day[ymd].assessment.overall_sleep_score)));
	    std::string s2 = "Deep       " +
		std::to_string(
		    static_cast<int>(std::round(sleep_by_day[ymd].assessment.deep_sleep_score)));
	    std::string s3 = "REM        " +
		std::to_string(
		    static_cast<int>(std::round(sleep_by_day[ymd].assessment.rem_sleep_score)));
	    std::string s4 = "Light      " +
		std::to_string(
		    static_cast<int>(std::round(sleep_by_day[ymd].assessment.light_sleep_score)));
	    std::string s5 = "Awakenings " +
		std::to_string(
		    static_cast<int>(std::round(sleep_by_day[ymd].assessment.awakenings_count)));
	    calendar.add(ymd, s1);
	    calendar.add(ymd, s2);
	    calendar.add(ymd, s3);
	    calendar.add(ymd, s4);
	    calendar.add(ymd, s5);
	}

	auto sys_days = std::chrono::sys_days(ymd);
	ymd = std::chrono::year_month_day(sys_days + std::chrono::days(1));
    }

    calendar.print();
}

inline void print_activities_stats(const std::string& h, const std::map<std::string, Activity>& a_list)
{
    cout << h << endl;
    cout << "-----------------------------------------------------" << endl;
    for (const auto& [sport, a] : a_list)
    {
	cout << std::left << std::setfill('.') << std::setw(20) << "Name"
	     << std::right << std::setfill('.') << std::setw(20)
	     << a.sport_profile_name << endl;
	cout << std::left << std::setfill('.') << std::setw(20) << "Activity"
	     << std::right << std::setfill('.') << std::setw(20)
	     << a.sport << " (" << a.sub_sport << ")" << endl;
	cout << std::left << std::setfill('.') << std::setw(20) << "Distance"
	     << std::right << std::setfill('.') << std::setw(20)
	     << formatted_number(a.total_distance / 1000) << " km" << endl;
	cout << std::left << std::setfill('.') << std::setw(20) << "Total elapsed time"
	     << std::right << std::setfill('.') << std::setw(20)
	     << formatted_number(a.total_elapsed_time / 60) << " min" << endl;
	cout << std::left << std::setfill('.') << std::setw(20) << "Ascent"
	     << std::right << std::setfill('.') << std::setw(20)
	     << formatted_number(a.total_ascent) << " m" << endl;
	cout << std::left << std::setfill('.') << std::setw(20) << "Descent"
	     << std::right << std::setfill('.') << std::setw(20)
	     << formatted_number(a.total_descent) << " m" << endl << endl;
    }
}

void ShellActivities::loop() const
{
    ushort option;

    do {
	system("clear");
	cout << "ACTIVITIES STATS" << endl;
	cout << "-------------------------------------------" << endl;
	cout << "1.- Dashboard" << endl;
	cout << "2.- Stats for a year" << endl;
	cout << "3.- Stats for a year/month" << endl;
	cout << "4.- Stats for a year/month/week" << endl;
	cout << "0.- Exit" << endl;
	cout << "Select an option: ";
	cin >> option;
	cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

	switch (option)
	{
	case 1:
	    this->dashboard();
	    cout << endl << "Press Enter to continue...";
	    cin.get();
	    break;
	case 2:
	    this->month_stats(ask_for_year());
	    cout << endl << "Press Enter to continue...";
	    cin.get();
	    break;
	case 3:
	    this->week_stats(ask_for_year(), ask_for_month());
	    cout << endl << endl << "Press Enter to continue...";
	    cin.get();
	    break;
	}
    } while (option != 0);
}

void ShellActivities::dashboard() const
{
    system("clear");
    cout << "ACTIVITIES STATS" << endl;
    cout << "-------------------------------------------" << endl;
    cout << "Years with activities: " << this->stats.get_data().size() << endl;

    for (const auto& [year, y_stats] : this->stats.get_data())
    {
	cout << endl << "AÑO " << year << endl;
	cout << "-------------------------------------------" << endl;
	std::map<std::string, int> activities_by_sport{};
	std::map<std::string, float> distances{};
	for (const auto& [month, m_stats] : y_stats.months)
	{
	    cout << MONTHS_NAMES[month] << endl;
	    std::map<std::string, int> month_activities_by_sport{};
	    std::map<std::string, float> month_distances{};
	    for (const auto& [week, w_stats] : m_stats.weeks)
	    {
		for (const auto& [day, d_stats] : w_stats.days)
		{
		    for (const auto& stats : d_stats.stats)
		    {
			if (stats.sport == "running" || stats.sport == "cycling" ||
			    stats.sport == "hiking" || stats.sport == "walking")
			{
			    distances[stats.sport] += stats.total_distance;
			    month_distances[stats.sport] += stats.total_distance;
			}
			activities_by_sport[stats.sport] += 1;
			month_activities_by_sport[stats.sport] += 1;
		    }
		}
	    }

	    for (const auto& [sport, count] : month_activities_by_sport)
	    {
		cout << "    " << sport << ": " << count << " activities";
		if (sport == "running" || sport == "cycling" || sport == "hiking" || sport == "walking")
		    cout << " | distance: " << formatted_number(month_distances[sport] / 1000) << " km";
		cout << endl;
	    }
	    cout << endl;
	}

	for (const auto& [sport, count] : activities_by_sport)
	{
	    cout << sport << ": " << count << " activities";
	    if (sport == "running" || sport == "cycling" || sport == "hiking" || sport == "walking")
		cout << " | distance: " << formatted_number(distances[sport] / 1000) << " km";
	    cout << endl;
	}
    }
}

void ShellActivities::month_stats(const ushort& year) const
{
    system("clear");
    cout << "MONTHLY ACTIVITIES STATS FOR YEAR " << year << endl;
    cout << "=====================================================" << endl;
    const auto& data = this->stats.get_data();
    if (!data.contains(year))
    {
	cout << "There are not data for this year" << endl;
	return;
    }

    for (const auto& [month, months] : data.at(year).months)
    {
	std::map<std::string, Activity> activities = std::accumulate(
	    months.weeks.begin(), months.weeks.end(), std::map<std::string, Activity>{},
	    [](const auto& acc, const auto& week_entry) {
		return std::accumulate(
		    week_entry.second.days.begin(), week_entry.second.days.end(), acc,
		    [](const auto& acc, const auto& day_entry) {
			return std::accumulate(
			    day_entry.second.stats.begin(), day_entry.second.stats.end(), acc,
			    [](const auto& acc, const auto& object) {
				auto c_acc{acc};
				c_acc[object.sport] += object;
				return c_acc;
			    });
		    });
	    });
	print_activities_stats(MONTHS_NAMES[month], activities);
    }
}

void ShellActivities::week_stats(const ushort& year, const ushort& month) const
{
    system("clear");
    cout << MONTHS_NAMES[month] << ", " << year << endl;
    cout << "=====================================================" << endl;

    const auto& years = this->stats.get_data();
    if (!years.contains(year))
    {
	cout << "There are not data for year " << year << endl;
	return;
    }

    const auto& ystats = years.at(year);
    if (!ystats.months.contains(month - 1))
    {
	cout << "There are not data for month " << MONTHS_NAMES[month] << endl;
	return;
    }

    const auto& mstats = ystats.months.at(month - 1);
    MonthStats<Activity> month_stats(mstats);

    // Complete first week with days off this month.
    if (ystats.months.contains(month - 2))
    {
	const auto& mstats_before = ystats.months.at(month - 2);
	for (auto itr = mstats_before.weeks.begin(); itr != mstats_before.weeks.end(); itr++)
	{
	    if (mstats.weeks.contains(itr->first))
	    {
		for (auto iday = itr->second.days.begin(); iday != itr->second.days.end(); iday++)
		{
		    month_stats.weeks[itr->first].days[iday->first] = iday->second;
		}
	    }
	}
    }

    // Complete last week with days off this month.
    if (ystats.months.contains(month))
    {
	const auto& mstats_after = ystats.months.at(month);
	for (auto itr = mstats_after.weeks.begin(); itr != mstats_after.weeks.end(); itr++)
	{
	    if (mstats.weeks.contains(itr->first))
	    {
		// cout << "ESTÁ: " << itr->first << endl;
		for (auto iday = itr->second.days.begin(); iday != itr->second.days.end(); iday++)
		{
		    month_stats.weeks[itr->first].days[iday->first] = iday->second;
		}
	    }
	}
    }

    std::map<std::chrono::year_month_day, std::vector<Activity>> activities_by_day{};
    for (auto itr = month_stats.weeks.begin(); itr != month_stats.weeks.end(); itr++)
    {
	for (auto itr_day = itr->second.days.begin();
	     itr_day != itr->second.days.end();
	     itr_day++)
	{
	    for (const auto& activity : itr_day->second.stats)
	    {
		std::istringstream stream({activity.start_time_utc});
		int year, month, day;
		char dash;

		stream >> year >> dash >> month >> dash >> day;
		if (!stream.fail() && !stream.bad())
		{
		    std::chrono::year_month_day ymd{
			std::chrono::year(year),
			std::chrono::month(month),
			std::chrono::day(day)};
		    activities_by_day[ymd].emplace_back(activity);
		}
	    }
	}
    }

    Calendar calendar(year, month);
    auto ymd = calendar.get_first_wd_ymd();
    auto last_ymd = calendar.get_last_wd_ymd();
    while (ymd <= last_ymd)
    {
	if (activities_by_day.contains(ymd))
	{
	    for (const auto& a : activities_by_day[ymd])
	    {
		if (a.total_distance > 0)
		    calendar.add(
			ymd,
			a.sport_profile_name + " (" + formatted_number(a.total_distance / 1000) + " km");
		else
		    calendar.add(
			ymd,
			a.sport_profile_name + " (" + formatted_number(a.total_elapsed_time / 60) + " min");
	    }
	}

	auto sys_days = std::chrono::sys_days(ymd);
	ymd = std::chrono::year_month_day(sys_days + std::chrono::days(1));
    }

    calendar.print();
}

} // namespace fitgalgo
