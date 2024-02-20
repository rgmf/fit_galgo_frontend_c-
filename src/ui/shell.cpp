#include <array>
#include <bits/types/time_t.h>
#include <chrono>
#include <concepts>
#include <cstddef>
#include <cstdio>
#include <format>
#include <functional>
#include <memory>
#include <numeric>
#include <string>
#include <iostream>
#include <sstream>
#include <filesystem>

#include <unistd.h>
#include <termios.h>

#include "../utils/string.h"
#include "../core/api.h"
#include "../core/stats.h"
#include "shell.h"
#include "calendar.h"
#include "repr.h"
#include "colors.h"
#include "tabular.h"

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

inline void print_header(const std::string& header)
{
    cout << colors::GREEN << colors::BOLD << header << endl;
    cout << "--------------------------------------------------------------------------------"
	 << colors::RESET << endl << endl;
}

inline void print_value(const std::string& label, const std::string& value)
{
    cout << std::left << std::setfill('.') << std::setw(40) << label
	 << std::right << std::setfill('.') << std::setw(40)
	 << value << endl;
}

inline std::string value_formatted(const std::string& label, const std::string& value, const size_t& w)
{
    size_t fill = w - (fitgalgo::mb_strlen(label) + fitgalgo::mb_strlen(value));

    std::stringstream ss;
    ss << label;
    for (size_t i = 0; i < fill; i++)
	ss << '.';
    ss << value;
    return ss.str();
}

inline std::string value_formatted(const std::string& label, const std::string& value)
{
    return value_formatted(label, value, 40);
}

template <typename T>
inline void print_optional_stat(
    const std::string& label, const std::optional<T> stat,
    std::function<std::string(const T&)> formatter)
{
    if (stat.has_value())
	cout << std::left << std::setfill('.') << std::setw(40) << label
	     << std::right << std::setfill('.') << std::setw(40)
	     << formatter(stat.value()) << endl;
}

void keyboard_handled(std::function<void()> callback)
{
    char c{};
    do
    {
	callback();
	cout << colors::YELLOW;
	cout << endl << endl;
	cout << "--------------------------------------------------" << endl;
	cout << "q -> exit" << endl;
	cout << "--------------------------------------------------" << endl;
	cout << colors::RESET;
	c = get_char();
    } while (c != 'q');
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
	    cout << colors::YELLOW;
	    cout << endl << endl;
	    cout << "--------------------------------------------------" << endl;
	    cout << "n -> next    p -> previous    q -> exit" << endl;
	    cout << "    :<year> -> jump to the year" << endl;
	    cout << "--------------------------------------------------" << endl;
	    cout << colors::RESET;
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
	 << unit(s.steps, "steps") << endl;
    cout << std::left << std::setfill('.') << std::setw(20) << "Distance"
	 << std::right << std::setfill('.') << std::setw(20)
	 << distance(s.distance) << endl;
    cout << std::left << std::setfill('.') << std::setw(20) << "Calories"
	 << std::right << std::setfill('.') << std::setw(20)
	 << calories(s.calories) << endl << endl;
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
	 << unit(s.assessment.overall_sleep_score / count) << endl
	 << std::left << std::setfill('.') << std::setw(20) << "Deep Score"
	 << std::right << std::setfill('.') << std::setw(20)
	 << unit(s.assessment.deep_sleep_score / count) << endl
	 << std::left << std::setfill('.') << std::setw(20) << "REM Score"
	 << std::right << std::setfill('.') << std::setw(20)
	 << unit(s.assessment.rem_sleep_score / count) << endl
	 << std::left << std::setfill('.') << std::setw(20) << "Light Score"
	 << std::right << std::setfill('.') << std::setw(20)
	 << unit(s.assessment.light_sleep_score / count) << endl
	 << std::left << std::setfill('.') << std::setw(20) << "Awekening Count"
	 << std::right << std::setfill('.') << std::setw(20)
	 << unit(s.assessment.awakenings_count / count) << endl
	 << std::left << std::setfill('.') << std::setw(20) << "Duration Score"
	 << std::right << std::setfill('.') << std::setw(20)
	 << unit(s.assessment.sleep_duration_score / count) << endl
	 << std::left << std::setfill('.') << std::setw(20) << "Quality Score"
	 << std::right << std::setfill('.') << std::setw(20)
	 << unit(s.assessment.sleep_quality_score / count) << endl
	 << std::left << std::setfill('.') << std::setw(20) << "Recovery Score"
	 << std::right << std::setfill('.') << std::setw(20)
	 << unit(s.assessment.sleep_recovery_score / count) << endl
	 << std::left << std::setfill('.') << std::setw(20) << "Avg. Stress"
	 << std::right << std::setfill('.') << std::setw(20)
	 << unit(s.assessment.average_stress_during_sleep / count) << endl << endl;
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

inline void print_activities_stats(const Activity* a)
{
    if (a->total_calories.has_value())
	cout << std::left << std::setfill('.') << std::setw(30) << "Total Calories"
	     << std::right << std::setfill('.') << std::setw(30)
	     << calories(a->total_calories.value()) << endl;
    if (a->avg_temperature.has_value())
	cout << std::left << std::setfill('.') << std::setw(30) << "Avg. Temperature"
	     << std::right << std::setfill('.') << std::setw(30)
	     << temperature(a->avg_temperature.value()) << endl;
    if (a->max_temperature.has_value())
	cout << std::left << std::setfill('.') << std::setw(30) << "Max. Temperature"
	     << std::right << std::setfill('.') << std::setw(30)
	     << temperature(a->max_temperature.value()) << endl;
    if (a->min_temperature.has_value())
	cout << std::left << std::setfill('.') << std::setw(30) << "Min. Temperature"
	     << std::right << std::setfill('.') << std::setw(30)
	     << temperature(a->min_temperature.value()) << endl;
    if (a->avg_respiration_rate.has_value())
	cout << std::left << std::setfill('.') << std::setw(30) << "Avg. Respiration Rate"
	     << std::right << std::setfill('.') << std::setw(30)
	     << unit(a->avg_respiration_rate.value()) << endl;
    if (a->max_respiration_rate.has_value())
	cout << std::left << std::setfill('.') << std::setw(30) << "Max. Respiration Rate"
	     << std::right << std::setfill('.') << std::setw(30)
	     << unit(a->max_respiration_rate.value()) << endl;
    if (a->min_respiration_rate.has_value())
	cout << std::left << std::setfill('.') << std::setw(30) << "Min. Respiration Rate"
	     << std::right << std::setfill('.') << std::setw(30)
	     << unit(a->min_respiration_rate.value()) << endl;
    if (a->training_load_peak.has_value())
	cout << std::left << std::setfill('.') << std::setw(30) << "Training Load Peak"
	     << std::right << std::setfill('.') << std::setw(30)
	     << unit(a->training_load_peak.value()) << endl;
    if (a->total_training_effect.has_value())
	cout << std::left << std::setfill('.') << std::setw(30) << "Total Training Effect"
	     << std::right << std::setfill('.') << std::setw(30)
	     << unit(a->total_training_effect.value()) << endl;
    if (a->total_anaerobic_training_effect.has_value())
	cout << std::left << std::setfill('.') << std::setw(30) << "Total Anaerobic Training Effect"
	     << std::right << std::setfill('.') << std::setw(30)
	     << unit(a->total_anaerobic_training_effect.value()) << endl;
}

inline std::vector<std::string> activities_stats(const Activity* a)
{
    std::vector<std::string> result{};
    
    if (a->total_calories.has_value())
	result.emplace_back(value_formatted("Calories", calories(a->total_calories.value()), 30));
    if (a->avg_temperature.has_value())
	result.emplace_back(value_formatted("Avg Temp", temperature(a->avg_temperature.value()), 30));
    if (a->max_temperature.has_value())
	result.emplace_back(value_formatted("Max Temp", temperature(a->max_temperature.value()), 30));
    if (a->min_temperature.has_value())
	result.emplace_back(value_formatted("Min Temp", temperature(a->min_temperature.value()), 30));
    if (a->avg_respiration_rate.has_value())
	result.emplace_back(value_formatted("Avg Resp", value(a->avg_respiration_rate.value()), 30));
    if (a->max_respiration_rate.has_value())
	result.emplace_back(value_formatted("Max Resp", value(a->max_respiration_rate.value()), 30));
    if (a->min_respiration_rate.has_value())
	result.emplace_back(value_formatted("Min Resp", value(a->min_respiration_rate.value()), 30));
    if (a->training_load_peak.has_value())
	result.emplace_back(value_formatted("Load Peak", value(a->training_load_peak.value()), 30));
    if (a->total_training_effect.has_value())
	result.emplace_back(value_formatted("Train Effect", value(a->total_training_effect.value()), 30));
    if (a->total_anaerobic_training_effect.has_value())
	result.emplace_back(
	    value_formatted("Anaerobic Effect", value(a->total_anaerobic_training_effect.value()), 30));
    return result;
}

inline void print_distance_activities_stats(const DistanceActivity* a)
{
    if (a->total_distance.has_value())
	cout << std::left << std::setfill('.') << std::setw(30) << "Distance"
	     << std::right << std::setfill('.') << std::setw(30)
	     << distance(a->total_distance.value()) << endl;
    if (a->avg_speed.has_value())
	cout << std::left << std::setfill('.') << std::setw(30) << "Avg. Speed"
	     << std::right << std::setfill('.') << std::setw(30)
	     << speed(a->avg_speed.value()) << endl;
    if (a->max_speed.has_value())
	cout << std::left << std::setfill('.') << std::setw(30) << "Max. Speed"
	     << std::right << std::setfill('.') << std::setw(30)
	     << speed(a->max_speed.value()) << endl;
    if (a->total_ascent.has_value())
	cout << std::left << std::setfill('.') << std::setw(30) << "Ascent"
	     << std::right << std::setfill('.') << std::setw(30)
	     << elevation(a->total_ascent.value()) << endl;
    if (a->total_descent.has_value())
	cout << std::left << std::setfill('.') << std::setw(30) << "Descent"
	     << std::right << std::setfill('.') << std::setw(30)
	     << elevation(a->total_descent.value()) << endl;

    print_activities_stats(a);
}

inline std::vector<std::string> distance_activities_stats(const DistanceActivity* a)
{
    std::vector<std::string> result{};
    if (a->total_distance.has_value())
    {
	std::stringstream ss;
	ss << std::left << std::setfill('.') << std::setw(30) << "Distance"
	   << std::right << std::setfill('.') << std::setw(30)
	   << distance(a->total_distance.value());
	result.emplace_back(ss.str());
    }
    if (a->avg_speed.has_value())
    {
	std::stringstream ss;
	ss << std::left << std::setfill('.') << std::setw(30) << "Avg. Speed"
	   << std::right << std::setfill('.') << std::setw(30)
	   << speed(a->avg_speed.value());
	result.emplace_back(ss.str());
    }
    if (a->max_speed.has_value())
    {
	std::stringstream ss;
	ss << std::left << std::setfill('.') << std::setw(30) << "Max. Speed"
	   << std::right << std::setfill('.') << std::setw(30)
	   << speed(a->max_speed.value());
	result.emplace_back(ss.str());
    }
    if (a->total_ascent.has_value())
    {
	std::stringstream ss;
	ss << std::left << std::setfill('.') << std::setw(30) << "Ascent"
	   << std::right << std::setfill('.') << std::setw(30)
	   << elevation(a->total_ascent.value());
	result.emplace_back(ss.str());
    }
    if (a->total_descent.has_value())
    {
	std::stringstream ss;
	ss << std::left << std::setfill('.') << std::setw(30) << "Descent"
	   << std::right << std::setfill('.') << std::setw(30)
	   << elevation(a->total_descent.value());
	result.emplace_back(ss.str());
    }

    auto as_res = activities_stats(a);
    result.insert(result.cend(), as_res.cbegin(), as_res.cend());

    return result;
}

inline void print_sets(const std::vector<Set>& sets)
{
    for (auto const& set : sets)
    {
	print_value("Timestamp", time(set.timestamp));
    }
}

inline std::vector<std::string> sets_formatted(const std::vector<Set>& sets)
{
    std::vector<std::string> result{};
    for (auto const& set : sets)
    {
	result.emplace_back(value_formatted("Timestamp", time(set.timestamp)));
    }
    return result;
}

inline void print_sets_activities_stats(const SetsActivity* a)
{
    if (!a->sets.empty())
	print_sets(a->sets);
    print_activities_stats(a);
}

inline std::vector<std::string> sets_activities_stats(const SetsActivity* a)
{
    std::vector<std::string> result{};
    
    if (!a->sets.empty())
	result = sets_formatted(a->sets);

    auto as_res = activities_stats(a);
    result.insert(result.cend(), as_res.cbegin(), as_res.cend());

    return result;
}

inline void print_splits(const std::vector<Split>& splits)
{
    for (auto const& split : splits)
    {
	print_value("Split Type", split.split_type);
    }
}

inline std::vector<std::string> splits_formatted(const std::vector<Split>& splits)
{
    std::vector<std::string> result{};
    for (auto const& split : splits)
    {
	result.emplace_back(value_formatted("Split Type", split.split_type));
    }
    return result;
}

inline void print_splits_activities_stats(const SplitsActivity* a)
{
    if (!a->splits.empty())
	print_splits(a->splits);
    print_activities_stats(a);
}

inline std::vector<std::string> splits_activities_stats(const SplitsActivity* a)
{
    std::vector<std::string> result{};
    
    if (!a->splits.empty())
	result = splits_formatted(a->splits);

    auto as_res = activities_stats(a);
    result.insert(result.cend(), as_res.cbegin(), as_res.cend());

    return result;
}

inline void print_aggregated_stats(const AggregatedStats& stats)
{
    const auto& a = stats.get_stats();

    cout << std::left << std::setfill('.') << std::setw(30) << "Number of activities"
	 << std::right << std::setfill('.') << std::setw(30)
	 << stats.get_count()<< endl;
    cout << std::left << std::setfill('.') << std::setw(30) << "From"
	 << std::right << std::setfill('.') << std::setw(30)
	 << date(stats.get_from_year_month_day()) << endl;
    cout << std::left << std::setfill('.') << std::setw(30) << "To"
	 << std::right << std::setfill('.') << std::setw(30)
	 << date(stats.get_to_year_month_day()) << endl;
    if (a->total_elapsed_time.has_value())
    {
	cout << std::left << std::setfill('.') << std::setw(30) << "Total elapsed time"
	     << std::right << std::setfill('.') << std::setw(30)
	     << time(a->total_elapsed_time.value()) << endl;
    }
    if (a->total_timer_time.has_value())
    {
	cout << std::left << std::setfill('.') << std::setw(30) << "Total timer time"
	     << std::right << std::setfill('.') << std::setw(30)
	     << time(a->total_timer_time.value()) << endl;
    }
    if (a->total_work_time.has_value())
    {
	cout << std::left << std::setfill('.') << std::setw(30) << "Total activity time"
	     << std::right << std::setfill('.') << std::setw(30)
	     << time(a->total_work_time.value()) << endl;
    }
    if (a->total_distance.has_value())
    {
	cout << std::left << std::setfill('.') << std::setw(30) << "Total distance"
	     << std::right << std::setfill('.') << std::setw(30)
	     << distance(a->total_distance.value()) << endl;
    }
    if (a->avg_speed.has_value())
	cout << std::left << std::setfill('.') << std::setw(30) << "Avg. Speed"
	     << std::right << std::setfill('.') << std::setw(30)
	     << speed(a->avg_speed.value()) << endl;
    if (a->max_speed.has_value())
	cout << std::left << std::setfill('.') << std::setw(30) << "Max. Speed"
	     << std::right << std::setfill('.') << std::setw(30)
	     << speed(a->max_speed.value()) << endl;
    if (a->total_ascent.has_value())
	cout << std::left << std::setfill('.') << std::setw(30) << "Ascent"
	     << std::right << std::setfill('.') << std::setw(30)
	     << elevation(a->total_ascent.value()) << endl;
    if (a->total_descent.has_value())
	cout << std::left << std::setfill('.') << std::setw(30) << "Descent"
	     << std::right << std::setfill('.') << std::setw(30)
	     << elevation(a->total_descent.value()) << endl;
    if (a->total_calories.has_value())
	cout << std::left << std::setfill('.') << std::setw(30) << "Total Calories"
	     << std::right << std::setfill('.') << std::setw(30)
	     << calories(a->total_calories.value()) << endl;
    if (a->avg_temperature.has_value())
	cout << std::left << std::setfill('.') << std::setw(30) << "Avg. Temperature"
	     << std::right << std::setfill('.') << std::setw(30)
	     << temperature(a->avg_temperature.value()) << endl;
    if (a->max_temperature.has_value())
	cout << std::left << std::setfill('.') << std::setw(30) << "Max. Temperature"
	     << std::right << std::setfill('.') << std::setw(30)
	     << temperature(a->max_temperature.value()) << endl;
    if (a->min_temperature.has_value())
	cout << std::left << std::setfill('.') << std::setw(30) << "Min. Temperature"
	     << std::right << std::setfill('.') << std::setw(30)
	     << temperature(a->min_temperature.value()) << endl;
    if (a->avg_respiration_rate.has_value())
	cout << std::left << std::setfill('.') << std::setw(30) << "Avg. Respiration Rate"
	     << std::right << std::setfill('.') << std::setw(30)
	     << unit(a->avg_respiration_rate.value()) << endl;
    if (a->max_respiration_rate.has_value())
	cout << std::left << std::setfill('.') << std::setw(30) << "Max. Respiration Rate"
	     << std::right << std::setfill('.') << std::setw(30)
	     << unit(a->max_respiration_rate.value()) << endl;
    if (a->min_respiration_rate.has_value())
	cout << std::left << std::setfill('.') << std::setw(30) << "Min. Respiration Rate"
	     << std::right << std::setfill('.') << std::setw(30)
	     << unit(a->min_respiration_rate.value()) << endl;
    if (a->training_load_peak.has_value())
	cout << std::left << std::setfill('.') << std::setw(30) << "Training Load Peak"
	     << std::right << std::setfill('.') << std::setw(30)
	     << unit(a->training_load_peak.value()) << endl;
    if (a->total_training_effect.has_value())
	cout << std::left << std::setfill('.') << std::setw(30) << "Total Training Effect"
	     << std::right << std::setfill('.') << std::setw(30)
	     << unit(a->total_training_effect.value()) << endl;
    if (a->total_anaerobic_training_effect.has_value())
	cout << std::left << std::setfill('.') << std::setw(30) << "Total Anaerobic Training Effect"
	     << std::right << std::setfill('.') << std::setw(30)
	     << unit(a->total_anaerobic_training_effect.value()) << endl;
}

ShellActivities::ShellActivities(const ActivitiesData& activities_data)
{
    const std::chrono::time_point now{std::chrono::system_clock::now()};
    const std::chrono::year_month_day ymd{std::chrono::floor<std::chrono::days>(now)};

    year = static_cast<int>(ymd.year());
    month = static_cast<unsigned>(ymd.month());
    data = activities_data;
}

void ShellActivities::loop()
{
    char c{};
    std::string action{};
    std::function<void()> callback = std::bind(&ShellActivities::month_stats, this);

    do
    {
	if (action.empty())
	{
	    system("clear");
	    switch (c)
	    {
	    case 'd':
		callback = std::bind(&ShellActivities::month_stats, this);
		break;
	    case 'y':
		callback = std::bind(&ShellActivities::year_stats, this);
		break;
	    case 'a':
		callback = std::bind(&ShellActivities::all_times_aggregated_stats, this);
		break;
	    }

	    callback();

	    cout << colors::YELLOW;
	    cout << endl << endl;
	    cout << "--------------------------------------------------------------------------------" << endl;
	    cout << "n -> next    p -> previous    q -> exit" << endl << endl;
	    cout << "Options Menu" << endl;
	    cout << "    d -> Dashboard" << endl;
	    cout << "    y -> Year Aggregated Stats" << endl;
	    cout << "    a -> All Times Aggregated Stats" << endl << endl;
	    cout << "Command Actions" << endl;
	    cout << "    :<year> -> jump to the year" << endl;
	    cout << "--------------------------------------------------------------------------------" << endl;
	    cout << colors::RESET;
	}

	c = get_char();
	
	if (action.empty())
	{
	    switch (c)
	    {
	    case 'n':
		if (month < 12 && month >= 1)
		{
		    month++;
		}
		else
		{
		    month = 1;
		    year++;
		}
		break;
	    case 'p':
		if (month <= 12 && month > 1)
		{
		    month--;
		}
		else
		{
		    month = 12;
		    year--;
		}
		break;
	    case ':':
		action = ":";
		break;
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

void ShellActivities::month_stats() const
{
    std::ostringstream oss;
    oss << "ACTIVITIES: MONTH DASHBOARD - " << year << ", " << MONTHS_NAMES[month - 1];
    print_header(oss.str());

    auto stats = AggregatedStats(year, month, this->data.activities);
    if (stats.empty())
    {
	cout << "There are not data for this date" << endl;
	return;
    }

    print_calendar();

    cout << endl;
    
    const auto& aggregated = stats.get_stats();

    print_optional_stat<float>("Training Load Peak", aggregated->training_load_peak, value);
    print_optional_stat<float>("Total Training Effect", aggregated->total_training_effect, value);
    print_optional_stat<float>(
	"Total Anaerobic Training Effect", aggregated->total_anaerobic_training_effect, value);

    cout << endl;

    auto summary = Tabular({"Distance", "Work Time", "Calories"});
    summary.add_row({
	    aggregated->total_distance.has_value() ? distance(aggregated->total_distance.value()) : "-",
	    aggregated->total_work_time.has_value() ? time(aggregated->total_work_time.value()) : "-",
	    aggregated->total_calories.has_value() ? calories(aggregated->total_calories.value()) : "-"});
    summary.print();
}

void ShellActivities::year_stats() const
{
    std::ostringstream oss;
    oss << "ACTIVITIES: YEAR DASHBOARD - " << year;
    print_header(oss.str());

    auto stats = AggregatedStats(year, this->data.activities);
    if (stats.empty())
    {
	cout << "There are not data for this date" << endl;
	return;
    }

    const auto& aggregated = stats.get_stats();

    print_value("Number of activities", std::to_string(stats.get_count()));

    cout << endl;

    print_optional_stat<float>("Training Load Peak", aggregated->training_load_peak, value);
    print_optional_stat<float>("Total Training Effect", aggregated->total_training_effect, value);
    print_optional_stat<float>(
	"Total Anaerobic Training Effect", aggregated->total_anaerobic_training_effect, value);

    cout << endl;

    auto summary = Tabular({"Distance", "Work Time", "Calories"});
    summary.add_row({
	    aggregated->total_distance.has_value() ? distance(aggregated->total_distance.value()) : "-",
	    aggregated->total_work_time.has_value() ? time(aggregated->total_work_time.value()) : "-",
	    aggregated->total_calories.has_value() ? calories(aggregated->total_calories.value()) : "-"});
    summary.print();

    cout << endl;

    auto tabular = Tabular();
    auto sport_stats = SportStats(year, this->data.activities);
    for (const auto& [sport, s_stats] : sport_stats.get_stats())
    {
	tabular.add_header(sport);
	if (auto distance_a = dynamic_cast<const DistanceActivity*>(s_stats.get_stats().get()))
	    tabular.add_values(sport, distance_activities_stats(distance_a));
	else if (auto sets_a = dynamic_cast<const SetsActivity*>(s_stats.get_stats().get()))
	    tabular.add_values(sport, sets_activities_stats(sets_a));
	else if (auto splits_a = dynamic_cast<const SplitsActivity*>(s_stats.get_stats().get()))
	    tabular.add_values(sport, splits_activities_stats(splits_a));
	else
	    tabular.add_values(sport, activities_stats(s_stats.get_stats().get()));
	/*
	cout << colors::BOLD << sport << colors::RESET << endl;
	if (auto distance_a = dynamic_cast<const DistanceActivity*>(s_stats.get_stats().get()))
	    print_distance_activities_stats(distance_a);
	else if (auto sets_a = dynamic_cast<const SetsActivity*>(s_stats.get_stats().get()))
	    print_sets_activities_stats(sets_a);
	else if (auto splits_a = dynamic_cast<const SplitsActivity*>(s_stats.get_stats().get()))
	    print_splits_activities_stats(splits_a);
	else
	    print_activities_stats(s_stats.get_stats().get());
	cout << endl;
	*/
    }
    tabular.print();
}

/*
inline void print_activities_stats(const std::string& h, const std::map<std::string, std::unique_ptr<Activity>>& activities)
{
    cout << h << endl;
    cout << "-----------------------------------------------------" << endl;
    for (const auto& [sport, a] : activities)
    {
	cout << std::left << std::setfill('.') << std::setw(30) << "Name"
	     << std::right << std::setfill('.') << std::setw(30)
	     << a->sport_profile_name << endl;
	cout << std::left << std::setfill('.') << std::setw(30) << "Type ID"
	     << std::right << std::setfill('.') << std::setw(30)
	     << a->get_id() << endl;
	cout << std::left << std::setfill('.') << std::setw(30) << "Activity"
	     << std::right << std::setfill('.') << std::setw(30)
	     << a->sport << " (" << a->sub_sport << ")" << endl;
	if (a->total_elapsed_time.has_value())
	{
	    cout << std::left << std::setfill('.') << std::setw(30) << "Total elapsed time"
		 << std::right << std::setfill('.') << std::setw(30)
		 << time(a->total_elapsed_time.value()) << endl;
	}
	if (auto distanceActivity = dynamic_cast<const DistanceActivity*>(a.get()))
	    print_distance_activities_stats(distanceActivity);
	cout << endl;
    }
}
*/

void ShellActivities::all_times_aggregated_stats() const
{
    system("clear");
    cout << "ACTIVITIES STATS" << endl;
    cout << "-------------------------------------------" << endl;

    const auto stats = AggregatedStats(this->data.activities);
    if (stats.empty())
    {
	cout << "There are not activities to show" << endl;
	return;
    }

    print_aggregated_stats(stats);
}

/*void ShellActivities::month_stats(const ushort& year, const ushort& month) const
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

    std::map<std::string, std::unique_ptr<Activity>> activities{};
    while (itr != this->data.activities.end() && (itr->first.year() == year && itr->first.month() == month))
    {
	if (!activities.contains(itr->second->sport))
	{
	    switch (itr->second->get_id())
	    {
	    case ActivityType::DISTANCE:
		activities[itr->second->sport] = std::make_unique<DistanceActivity>();
		break;
	    case ActivityType::SETS:
		activities[itr->second->sport] = std::make_unique<SetsActivity>();
		break;
	    case ActivityType::SPLITS:
		activities[itr->second->sport] = std::make_unique<SplitsActivity>();
		break;
	    default:
		activities[itr->second->sport] = std::make_unique<Activity>();
		break;
	    }
	}
	activities[itr->second->sport]->merge(itr->second);
	itr++;
    }

    print_activities_stats(MONTHS_NAMES[month - 1], activities);
    }*/

void ShellActivities::print_calendar() const
{
    Calendar calendar{year, month};
    auto first_wd_ymd = calendar.get_first_wd_ymd();
    auto last_wd_ymd = calendar.get_last_wd_ymd();

    auto itr = std::find_if(
	this->data.activities.cbegin(),
	this->data.activities.cend(),
	[this, first_wd_ymd](const auto& item) {
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
	if (itr->second->get_id() == ActivityType::DISTANCE)
	    s = itr->second->sport_profile_name +
		" (" + distance(itr->second->total_distance.value()) + ")";
	else
	    s = itr->second->sport_profile_name +
		 " (" + time(itr->second->total_elapsed_time.value()) + ")";
	calendar.add(itr->first.ymd(), s);
	itr++;
    }

    calendar.print();
}

} // namespace fitgalgo
