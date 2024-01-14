#include <bits/chrono.h>
#include <chrono>
#include <cstddef>
#include <cstdio>
#include <format>
#include <functional>
#include <numeric>
#include <string>
#include <iostream>
#include <sstream>
#include <filesystem>

#include <unistd.h>
#include <termios.h>

#include "../core/api.h"
#include "shell.h"
#include "calendar.h"

using std::cin;
using std::cout;
using std::endl;

namespace fitgalgo
{

char get_char()
{
    // Terminal to raw mode ON
    system("stty raw");
    char c = getchar();
    system("stty cooked");
    // Terminal to row mode OFF
    return c;
}

inline void disable_echo()
{
    struct termios tty;
    tcgetattr(STDIN_FILENO, &tty);
    tty.c_lflag &= ~ECHO;
    tcsetattr(STDIN_FILENO, TCSANOW, &tty);
}

inline void enable_echo()
{
    struct termios tty;
    tcgetattr(STDIN_FILENO, &tty);
    tty.c_lflag |= ECHO;
    tcsetattr(STDIN_FILENO, TCSANOW, &tty);
}

void keyboard_handled(
    std::function<void(const ushort&, const ushort&)> callback,
    ushort year,
    ushort month)
{
    char c{};
    std::string action{};

    do
    {
	if (action.empty())
	{
	    callback(year, month);
	    cout << "\033[33m";
	    cout << endl << endl;
	    cout << "--------------------------------------------------" << endl;
	    cout << "n -> next    p -> previous    q -> exit" << endl;
	    cout << "    :<year> -> jump to the year" << endl;
	    cout << "--------------------------------------------------" << endl;
	    cout << "\033[0m";
	}

	c = get_char();

	if (action.empty())
	{
	    switch (c)
	    {
	    case 'n': month < 12 && month >= 1 ? month++ : month; break;
	    case 'p': month <= 12 && month > 1 ? month-- : month; break;
	    case ':': action = ":"; break;
	    }
	}
	else
	{
	    if (c == 27)
	    {
		action = {};
	    }
	    else if (c >= '0' && c <= '9')
	    {
		action += c;
	    }
	    else if (c == 13)
	    {
		int new_year = std::atoi(action.substr(1, action.length() - 1).c_str());
		year = new_year > 0 ? new_year : year;
		action = {};
	    }
	}

    } while (c != 'q');
}

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

inline bool Shell::login()
{
    Result<LoginData> login_result;
    std::string username;
    std::string password;
 
    system("clear");
    cout << "You need to login to Fit Galgo API" << endl;
    cout << endl << "Username: ";
    std::cin >> username;
    cout << "Password: ";
    disable_echo();
    std::cin >> password;
    enable_echo();
    cout << endl;
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    login_result = this->connection.login(username, password);
    if (login_result.is_valid())
    {
	cout << "Login okay" << endl;
	return true;
    }
    else
    {
	std::cerr << "Login error" << endl;
	std::cerr << login_result.get_error().error_to_string() << endl;
	return false;
    }
}

inline void Shell::upload_path() const
{
    system("clear");
    try
    {
	std::filesystem::path path;
	do
	{
	    cout << endl << "File path: ";
	    std::cin >> path;
	    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
	} while (!std::filesystem::exists(path));

	auto results = this->connection.post_file(path);
	unsigned short total = 0;
	unsigned short accepted = 0;
	for (auto& result : results)
	{
	    if (result.is_valid())
	    {
		const auto& ufd = result.get_data();
		for (const auto& uf : ufd.uploaded_files)
		{
		    total++;
		    cout << "File: " << uf.filename
			    << " | accepted: " << uf.accepted
			    << endl;
		    if (!uf.errors.empty())
			cout << "Errors:" << endl;
		    else
			accepted++;

		    for (auto& error : uf.errors)
		    {
			cout << error << endl;
		    }
		}
	    }
	    else
	    {
		std::cerr << result.get_error().error_to_string() << endl;
	    }
	}
	cout << endl << "TOTAL: " << total << endl;
	cout << "ACCEPTED: " << accepted << endl << endl;
    }
    catch (std::filesystem::filesystem_error& error)
    {
	std::cerr << "EXCEPTION: " << error.what() << endl;
	cout << "Try again..." << endl;
    }

    cout << endl << "Press Enter to continue...";
    std::cin.get();
}

inline void Shell::steps() const
{
    try
    {
	auto result = this->connection.get_steps();
	if (result.is_valid())
	{
	    auto ui = fitgalgo::ShellSteps(result.get_data());
	    ui.loop();
	}
	else
	{
	    std::cerr << result.get_error().error_to_string() << endl;
	    cout << endl << "Press Enter to continue...";
	    std::cin.get();
	}
    }
    catch (const std::exception& e) {
	std::cerr << "Error: " << e.what() << endl;
	cout << endl << "Press Enter to continue...";
	std::cin.get();
    }
}

inline void Shell::sleep() const
{
    try
    {
	auto result = this->connection.get_sleep();
	if (result.is_valid())
	{
	    auto ui = fitgalgo::ShellSleep(result.get_data());
	    ui.loop();
	}
	else
	{
	    std::cerr << result.get_error().error_to_string() << endl;
	    cout << endl << "Press Enter to continue...";
	    std::cin.get();
	}
    }
    catch (const std::exception& e) {
	std::cerr << "Error: " << e.what() << endl;
	cout << endl << "Press Enter to continue...";
	std::cin.get();
    }
}

inline void Shell::activities() const
{
    try
    {
	auto result = this->connection.get_activities();
	if (result.is_valid())
	{
	    auto ui = fitgalgo::ShellActivities(result.get_data());
	    ui.loop();
	}
	else
	{
	    std::cerr << result.get_error().error_to_string() << endl;
	    cout << endl << "Press Enter to continue...";
	    std::cin.get();
	}
    }
    catch (const std::exception& e) {
	std::cerr << "Error: " << e.what() << endl;
	cout << endl << "Press Enter to continue...";
	std::cin.get();
    }
}

void Shell::loop()
{
    char option{};

    do
    {
	system("clear");
	cout << "MENU" << endl;
	cout << "-------------------------------------------" << endl;
	if (!this->connection.has_token())
	{
	    option = '0';
	}
	else
	{
	    cout << "1 - Logout" << endl;
	    cout << "2 - Upload file" << endl;
	    cout << "3 - Steps" << endl;
	    cout << "4 - Sleep" << endl;
	    cout << "5 - Activities" << endl;
	    cout << "q - Exit" << endl;
	    cout << "Select an option: ";

	    option = get_char();
	}

	switch (option)
	{
	case '0':
	    if (!this->login())
	    {
		std::cout << "Press 'q' to exit or 'enter' to continue...";
		option = get_char();
	    }
	    break;
	case '1':
	    this->connection.logout();
	    break;
	case '2':
	    this->upload_path();
	    break;
	case '3':
	    this->steps();
	    break;
	case '4':
	    this->sleep();
	    break;
	case '5':
	    this->activities();
	    break;
	case 'q':
	    system("clear");
	    cout << "Are you sure you want to exit from the application [s/n]: ";
	    option = get_char();
	    option = option == 's' || option == 'S' ? 'q' : 'n';
	    break;
	}
    } while (option != 'q');

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
    char option;
    const std::chrono::time_point now{std::chrono::system_clock::now()};
    const std::chrono::year_month_day ymd{std::chrono::floor<std::chrono::days>(now)};

    do {
	system("clear");
	cout << "STEPS STATS" << endl;
	cout << "-------------------------------------------" << endl;
	cout << "1 - All aggregated stats" << endl;
	cout << "2 - Stats for a year" << endl;
	cout << "3 - Stats for a year/month" << endl;
	cout << "4 - Stats for a year/month/week" << endl;
	cout << "q - Exit" << endl;
	cout << "Select an option: ";

	option = get_char();
	
	switch (option)
	{
	case '1':
	    this->year_stats();
	    cout << endl << "Press Enter to continue...";
	    cin.get();
	    break;
	case '2':
	    cout << endl;
	    keyboard_handled([this](const ushort& year, const ushort& month) {
		this->month_stats(year, month);
	    }, static_cast<int>(ymd.year()), static_cast<unsigned>(ymd.month()));
	    break;
	case '3':
	    cout << endl;
	    keyboard_handled([this](const ushort& year, const ushort& month) {
		this->week_stats(year, month);
	    }, static_cast<int>(ymd.year()), static_cast<unsigned>(ymd.month()));
	    break;
	}
    } while (option != 'q');
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

    short current_year = this->data.steps.begin()->first.year();
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

void ShellSteps::month_stats(const ushort& year, const ushort& month) const
{
    system("clear");
    cout << "MONTHLY STEPS STATS FOR " << year << ", " << MONTHS_NAMES[month - 1] << endl;
    cout << "=====================================================" << endl;

    auto itr = std::find_if(
	this->data.steps.cbegin(),
	this->data.steps.cend(),
	[year, month](const auto& item) {
	    return item.first.year() == year && item.first.month() == month;
	});
    if (itr == this->data.steps.end())
    {
	cout << "There are not data for this date" << endl;
	return;
    }

    Steps all_steps{};
    while (itr != this->data.steps.end() && (itr->first.year() == year && itr->first.month() == month))
    {
	all_steps += itr->second;
	itr++;
    }

    print_steps_stats(MONTHS_NAMES[month - 1], all_steps);
}

void ShellSteps::week_stats(const ushort& year, const ushort& month) const
{
    system("clear");
    cout << MONTHS_NAMES[month - 1] << ", " << year << endl;
    cout << "=====================================================" << endl;

    Calendar calendar{year, month};
    auto first_wd_ymd = calendar.get_first_wd_ymd();
    auto last_wd_ymd = calendar.get_last_wd_ymd();

    auto itr = std::find_if(
	this->data.steps.cbegin(),
	this->data.steps.cend(),
	[year, month, first_wd_ymd](const auto& item) {
	    return item.first.ymd() == first_wd_ymd ||
		(item.first.year() == year && item.first.month() == month);
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
    char option;
    const std::chrono::time_point now{std::chrono::system_clock::now()};
    const std::chrono::year_month_day ymd{std::chrono::floor<std::chrono::days>(now)};

    do {
	system("clear");
	cout << "SLEEP STATS" << endl;
	cout << "-------------------------------------------" << endl;
	cout << "1 - All aggregated stats" << endl;
	cout << "2 - Stats for a year" << endl;
	cout << "3 - Stats for a year/month" << endl;
	cout << "4 - Stats for a year/month/week" << endl;
	cout << "q - Exit" << endl;
	cout << "Select an option: ";

	option = get_char();

	switch (option)
	{
	case '1':
	    this->year_stats();
	    cout << endl << "Press Enter to continue...";
	    cin.get();
	    break;
	case '2':
	    cout << endl;
	    keyboard_handled([this](const ushort& year, const ushort& month) {
		this->month_stats(year, month);
	    }, static_cast<int>(ymd.year()), static_cast<unsigned>(ymd.month()));
	    break;
	case '3':
	    cout << endl;
	    keyboard_handled([this](const ushort& year, const ushort& month) {
		this->week_stats(year, month);
	    }, static_cast<int>(ymd.year()), static_cast<unsigned>(ymd.month()));
	    break;
	}
    } while (option != 'q');
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

    short current_year = this->data.sleep.begin()->first.year();
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

void ShellSleep::month_stats(const ushort& year, const ushort& month) const
{
    system("clear");
    cout << "MONTHLY SLEEP STATS " << year << ", " << MONTHS_NAMES[month - 1] << endl;
    cout << "=====================================================" << endl;

    auto itr = std::find_if(
	this->data.sleep.cbegin(),
	this->data.sleep.cend(),
	[year, month](const auto& item) {
	    return item.first.year() == year && item.first.month() == month;
	});
    if (itr == this->data.sleep.end())
    {
	cout << "There are not data for this year" << endl;
	return;
    }

    SleepWithCount swc{};
    while (itr != this->data.sleep.end() && (itr->first.year() == year && itr->first.month() == month))
    {
	swc = SleepWithCount{swc.sleep + itr->second, swc.count + 1};
	itr++;
    }

    print_sleep_stats(MONTHS_NAMES[month - 1], swc.sleep, swc.count);
}

void ShellSleep::week_stats(const ushort& year, const ushort& month) const
{
    system("clear");
    cout << MONTHS_NAMES[month - 1] << ", " << year << endl;
    cout << "=====================================================" << endl;

    Calendar calendar{year, month};
    auto first_wd_ymd = calendar.get_first_wd_ymd();
    auto last_wd_ymd = calendar.get_last_wd_ymd();

    auto itr = std::find_if(
	this->data.sleep.cbegin(),
	this->data.sleep.cend(),
	[year, month, first_wd_ymd](const auto& item) {
	    return item.first.ymd() == first_wd_ymd ||
		(item.first.year() == year && item.first.month() == month);
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
    char option;
    const std::chrono::time_point now{std::chrono::system_clock::now()};
    const std::chrono::year_month_day ymd{std::chrono::floor<std::chrono::days>(now)};
    
    do {
	system("clear");
	cout << "ACTIVITIES STATS" << endl;
	cout << "-------------------------------------------" << endl;
	cout << "1 - All aggregated stats" << endl;
	cout << "2 - Stats for a year" << endl;
	cout << "3 - Stats for a year/month" << endl;
	cout << "4 - Stats for a year/month/week" << endl;
	cout << "q - Exit" << endl;
	cout << "Select an option: ";

	option = get_char();

	switch (option)
	{
	case '1':
	    this->dashboard();
	    cout << endl << "Press Enter to continue...";
	    cin.get();
	    break;
	case '2':
	    cout << endl;
	    keyboard_handled([this](const ushort& year, const ushort& month) {
		this->month_stats(year, month);
	    }, static_cast<int>(ymd.year()), static_cast<unsigned>(ymd.month()));
	    break;
	case '3':
	    cout << endl;
	    keyboard_handled([this](const ushort& year, const ushort& month) {
		this->week_stats(year, month);
	    }, static_cast<int>(ymd.year()), static_cast<unsigned>(ymd.month()));
	    break;
	}
    } while (option != 'q');
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
    short current_year = this->data.activities.begin()->first.year();
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

void ShellActivities::month_stats(const ushort& year, const ushort& month) const
{
    system("clear");
    cout << "MONTHLY ACTIVITIES STATS FOR " << year << ", " << MONTHS_NAMES[month - 1] << endl;
    cout << "=====================================================" << endl;

    auto itr = std::find_if(
	this->data.activities.cbegin(),
	this->data.activities.cend(),
	[year, month](const auto& item) {
	    return item.first.year() == year && item.first.month() == month;
	});
    if (itr == this->data.activities.end())
    {
	cout << "There are not data for this year" << endl;
	return;
    }

    std::map<std::string, Activity> activities{};
    while (itr != this->data.activities.end() && (itr->first.year() == year && itr->first.month() == month))
    {
	activities[itr->second.sport] += itr->second;
	itr++;
    }

    print_activities_stats(MONTHS_NAMES[month - 1], activities);
}

void ShellActivities::week_stats(const ushort& year, const ushort& month) const
{
    system("clear");
    cout << MONTHS_NAMES[month - 1] << ", " << year << endl;
    cout << "=====================================================" << endl;

    Calendar calendar{year, month};
    auto first_wd_ymd = calendar.get_first_wd_ymd();
    auto last_wd_ymd = calendar.get_last_wd_ymd();

    auto itr = std::find_if(
	this->data.activities.cbegin(),
	this->data.activities.cend(),
	[year, month, first_wd_ymd](const auto& item) {
	    return item.first.ymd() == first_wd_ymd ||
		(item.first.year() == year && item.first.month() == month);
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
