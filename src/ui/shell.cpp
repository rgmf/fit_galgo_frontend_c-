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
	 << formatted_number(s.distance) << " m" << endl;
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
                                return acc + day_entry.second;
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
			return acc + day_entry.second;
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
	    std::istringstream stream(itr_day->second.datetime_local);
	    int year, month, day;
	    char dash;

	    stream >> year >> dash >> month >> dash >> day;
	    if (!stream.fail() && !stream.bad())
	    {
		std::chrono::year_month_day ymd{
		    std::chrono::year(year),
		    std::chrono::month(month),
		    std::chrono::day(day)};
		steps_by_day[ymd] = itr_day->second;
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
	    std::string s2 = std::to_string(steps_by_day[ymd].distance) + " m";
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
	 << formatted_number(s.assessment.overall_sleep_score / count) << endl << endl;
    // << formatted_number(static_cast<int>(s.assessment.overall_sleep_score / count)) << endl << endl;
    // cout << std::left << std::setfill('.') << std::setw(20) << "Distance"
    // 	 << std::right << std::setfill('.') << std::setw(20)
    // 	 << formatted_number(s.distance) << " m" << endl;
    // cout << std::left << std::setfill('.') << std::setw(20) << "Calories"
    // 	 << std::right << std::setfill('.') << std::setw(20)
    // 	 << formatted_number(s.calories) << " kcal" << endl << endl;
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
                                return SleepWithCount{acc.sleep + day_entry.second, acc.count + 1};
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
			return SleepWithCount{acc.sleep + day_entry.second, acc.count + 1};
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
		// cout << "ESTÁ: " << itr->first << endl;
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
	    std::string dtl{itr_day->second.datetime_local()};
	    cout << "dtl: " << dtl << endl << endl;
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
		sleep_by_day[ymd] = itr_day->second;
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
	    std::string s1 = std::to_string(sleep_by_day[ymd].assessment.overall_sleep_score);
	    calendar.add(ymd, s1);
	}

	auto sys_days = std::chrono::sys_days(ymd);
	ymd = std::chrono::year_month_day(sys_days + std::chrono::days(1));
    }

    calendar.print();
}

} // namespace fitgalgo
