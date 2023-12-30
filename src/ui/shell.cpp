#include <chrono>
#include <cstddef>
#include <format>
#include <numeric>
#include <string>
#include <iostream>
#include <sstream>

#include "shell.h"
#include "calendar.h"

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

    if (data.steps.empty())
    {
	cout << "There are not steps data to show" << endl;
	return;
    }

    std::string current_year = this->data.steps.begin()->first.year();
    Steps all_steps{};
    for (const auto& [idx, steps] : this->data.steps)
    {
	if (current_year != idx.year())
	{
	    print_steps_stats(std::format("Year {}", current_year), all_steps);
	    current_year = idx.year();
	    all_steps = Steps{};
	}
	all_steps += steps;
    }

    print_steps_stats(std::format("Year {}", current_year), all_steps);
}

void ShellSteps::month_stats(const ushort& year) const
{
    system("clear");
    cout << "MONTHLY STEPS STATS FOR YEAR " << year << endl;
    cout << "=====================================================" << endl;

    const auto year_str = std::to_string(year);

    auto itr = std::find_if(
	this->data.steps.cbegin(),
	this->data.steps.cend(),
	[year_str](const auto& item) { return item.first.year() == year_str; });

    if (itr == this->data.steps.end())
    {
	cout << "There are not data for this year" << endl;
	return;
    }

    std::string current_month = itr->first.month();
    Steps all_steps{};
    while (itr != this->data.steps.end() && itr->first.year() == year_str)
    {
	if (current_month != itr->first.month())
	{
	    print_steps_stats(MONTHS_NAMES[std::atoi(current_month.c_str()) - 1], all_steps);
	    current_month = itr->first.month();
	    all_steps = Steps{};
	}
	all_steps += itr->second;
	itr++;
    }

    print_steps_stats(MONTHS_NAMES[std::atoi(current_month.c_str()) - 1], all_steps);
}

void ShellSteps::week_stats(const ushort& year, const ushort& month) const
{
    system("clear");
    cout << MONTHS_NAMES[month - 1] << ", " << year << endl;
    cout << "=====================================================" << endl;

    Calendar calendar{year, month};
    auto first_wd_ymd = calendar.get_first_wd_ymd();
    auto last_wd_ymd = calendar.get_last_wd_ymd();

    auto year_str = std::to_string(year);
    auto month_str = std::to_string(month);
    auto itr = std::find_if(
	this->data.steps.cbegin(),
	this->data.steps.cend(),
	[year_str, month_str, first_wd_ymd](const auto& item) {
	    return item.first.ymd() == first_wd_ymd ||
		(item.first.year() == year_str && item.first.month() == month_str);
	});
    if (itr == this->data.steps.end())
    {
     	cout << "There are not data for this date " << endl;
     	return;
    }

    while (itr != this->data.steps.end() && itr->first.ymd() <= last_wd_ymd)
    {
	std::string s1 = std::to_string(itr->second.steps) + " steps";
	std::string s2 = std::to_string(
	    static_cast<int>(std::round(itr->second.distance))) + " m";
	std::string s3 = std::to_string(itr->second.calories) + " kcal";

	calendar.add(itr->first.ymd(), s1);
	calendar.add(itr->first.ymd(), s2);
	calendar.add(itr->first.ymd(), s3);
	itr++;
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

    if (this->data.sleep.empty())
    {
	cout << "There are not sleep data to show" << endl;
	return;
    }

    std::string current_year = this->data.sleep.begin()->first.year();
    SleepWithCount swc{};
    for (const auto& [idx, sleep] : this->data.sleep)
    {
	if (current_year != idx.year())
	{
	    print_sleep_stats(std::format("Year {}", current_year), swc.sleep, swc.count);
	    current_year = idx.year();
	    swc = SleepWithCount{};
	}
	swc = SleepWithCount{swc.sleep + sleep, swc.count + 1};
    }

    print_sleep_stats(std::format("Year {}", current_year), swc.sleep, swc.count);
}

void ShellSleep::month_stats(const ushort& year) const
{
    system("clear");
    cout << "MONTHLY SLEEP STATS FOR YEAR " << year << endl;
    cout << "=====================================================" << endl;

    const auto year_str = std::to_string(year);

    auto itr = std::find_if(
	this->data.sleep.cbegin(),
	this->data.sleep.cend(),
	[year_str](const auto& item) { return item.first.year() == year_str; });
    if (itr == this->data.sleep.end())
    {
	cout << "There are not data for this year" << endl;
	return;
    }

    std::string current_month = itr->first.month();
    SleepWithCount swc{};
    while (itr != this->data.sleep.end() && itr->first.year() == year_str)
    {
	if (current_month != itr->first.month())
	{
	    print_sleep_stats(MONTHS_NAMES[std::atoi(current_month.c_str()) - 1], swc.sleep, swc.count);
	    current_month = itr->first.month();
	    swc = SleepWithCount{};
	}
	swc = SleepWithCount{swc.sleep + itr->second, swc.count + 1};
	itr++;
    }

    print_sleep_stats(MONTHS_NAMES[std::atoi(current_month.c_str()) - 1], swc.sleep, swc.count);
}

void ShellSleep::week_stats(const ushort& year, const ushort& month) const
{
    system("clear");
    cout << MONTHS_NAMES[month - 1] << ", " << year << endl;
    cout << "=====================================================" << endl;

    Calendar calendar{year, month};
    auto first_wd_ymd = calendar.get_first_wd_ymd();
    auto last_wd_ymd = calendar.get_last_wd_ymd();

    auto year_str = std::to_string(year);
    auto month_str = std::to_string(month);
    auto itr = std::find_if(
	this->data.sleep.cbegin(),
	this->data.sleep.cend(),
	[year_str, month_str, first_wd_ymd](const auto& item) {
	    return item.first.ymd() == first_wd_ymd ||
		(item.first.year() == year_str && item.first.month() == month_str);
	});
    if (itr == this->data.sleep.end())
    {
     	cout << "There are not data for this date " << endl;
     	return;
    }

    while (itr != this->data.sleep.end() && itr->first.ymd() <= last_wd_ymd)
    {
	std::string s1 = "Overall    " +
	    std::to_string(
		static_cast<int>(std::round(itr->second.assessment.overall_sleep_score)));
	std::string s2 = "Deep       " +
	    std::to_string(
		static_cast<int>(std::round(itr->second.assessment.deep_sleep_score)));
	std::string s3 = "REM        " +
	    std::to_string(
		static_cast<int>(std::round(itr->second.assessment.rem_sleep_score)));
	std::string s4 = "Light      " +
	    std::to_string(
		static_cast<int>(std::round(itr->second.assessment.light_sleep_score)));
	std::string s5 = "Awakenings " +
	    std::to_string(
		static_cast<int>(std::round(itr->second.assessment.awakenings_count)));

	if (itr->second.is_early_morning())
	{
	    auto ymd = itr->first.ymd();

	    auto yesterday = std::chrono::year_month_day(
		std::chrono::sys_days(ymd) - std::chrono::days(1));

	    calendar.add(yesterday, s1);
	    calendar.add(yesterday, s2);
	    calendar.add(yesterday, s3);
	    calendar.add(yesterday, s4);
	    calendar.add(yesterday, s5);
	}
	else
	{
	    calendar.add(itr->first.ymd(), s1);
	    calendar.add(itr->first.ymd(), s2);
	    calendar.add(itr->first.ymd(), s3);
	    calendar.add(itr->first.ymd(), s4);
	    calendar.add(itr->first.ymd(), s5);
	}

	itr++;
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

inline void print_activities_stats(const std::string& header, const Activity& a)
{
    cout << header << " | " << a.total_timer_time << endl;
}

void ShellActivities::dashboard() const
{
    system("clear");
    cout << "ACTIVITIES STATS" << endl;
    cout << "-------------------------------------------" << endl;

    if (this->data.activities.empty())
    {
	cout << "There are not activities to show" << endl;
	return;
    }

    std::map<std::string, Activity> activities{};
    std::string current_year = this->data.activities.begin()->first.year();
    for (const auto& [idx, a] : this->data.activities)
    {
	if (current_year != idx.year())
	{
	    print_activities_stats(std::format("Year {}", current_year), activities);
	    current_year = idx.year();
	    activities = {};
	}

	activities[a.sport] += a;
    }

    print_activities_stats(std::format("Year {}", current_year), activities);
}

void ShellActivities::month_stats(const ushort& year) const
{
    system("clear");
    cout << "MONTHLY ACTIVITIES STATS FOR YEAR " << year << endl;
    cout << "=====================================================" << endl;

    const auto year_str = std::to_string(year);

    auto itr = std::find_if(
	this->data.activities.cbegin(),
	this->data.activities.cend(),
	[year_str](const auto& item) { return item.first.year() == year_str; });
    if (itr == this->data.activities.end())
    {
	cout << "There are not data for this year" << endl;
	return;
    }

    std::string current_month = itr->first.month();
    std::map<std::string, Activity> activities{};
    while (itr != this->data.activities.end() && itr->first.year() == year_str)
    {
	if (current_month != itr->first.month())
	{
	    print_activities_stats(MONTHS_NAMES[std::atoi(current_month.c_str()) - 1], activities);
	    current_month = itr->first.month();
	    activities = {};
	}
	activities[itr->second.sport] += itr->second;
	itr++;
    }

    print_activities_stats(MONTHS_NAMES[std::atoi(current_month.c_str()) - 1], activities);
}

void ShellActivities::week_stats(const ushort& year, const ushort& month) const
{
    system("clear");
    cout << MONTHS_NAMES[month - 1] << ", " << year << endl;
    cout << "=====================================================" << endl;

    Calendar calendar{year, month};
    auto first_wd_ymd = calendar.get_first_wd_ymd();
    auto last_wd_ymd = calendar.get_last_wd_ymd();

    auto year_str = std::to_string(year);
    auto month_str = std::to_string(month);
    auto itr = std::find_if(
	this->data.activities.cbegin(),
	this->data.activities.cend(),
	[year_str, month_str, first_wd_ymd](const auto& item) {
	    return item.first.ymd() == first_wd_ymd ||
		(item.first.year() == year_str && item.first.month() == month_str);
	});
    if (itr == this->data.activities.end())
    {
     	cout << "There are not data for this date " << endl;
     	return;
    }

    while (itr != this->data.activities.end() && itr->first.ymd() <= last_wd_ymd)
    {
	std::string s{};
	if (itr->second.total_distance > 0)
	    s = itr->second.sport_profile_name +
		" (" + formatted_number(itr->second.total_distance / 1000) + " km";
	else
	    s = itr->second.sport_profile_name +
		" (" + formatted_number(itr->second.total_elapsed_time / 60) + " min";
	calendar.add(itr->first.ymd(), s);
	itr++;
    }

    calendar.print();
}

} // namespace fitgalgo
