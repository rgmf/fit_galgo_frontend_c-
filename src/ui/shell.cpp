#include <ranges>
#include <cstddef>
#include <string>
#include <iostream>
#include <sstream>

#include <unistd.h>
#include <termios.h>

#include "shell.h"
#include "calendar.h"
#include "repr.h"
#include "colors.h"
#include "tabular.h"
#include "printer.h"
#include "../core/stats.h"

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

void ShellStats::loop()
{
    char c{};
    std::string action{};
    std::function<void()> callback = std::bind(&ShellStats::month_stats, this);
    std::optional<std::pair<size_t, size_t>> limits = std::pair<size_t, size_t>{1, 12};
    ushort* next_prev_ref = &month;

    do
    {
	if (action.empty())
	{
	    system("clear");
	    switch (c)
	    {
	    case 'd':
		next_prev_ref = &month;
		limits = std::pair<size_t, size_t>{1, 12};
		callback = std::bind(&ShellStats::month_stats, this);
		break;
	    case 'y':
		next_prev_ref = &year;
		limits = {};
		callback = std::bind(&ShellStats::year_stats, this);
		break;
	    case 'a':
		next_prev_ref = nullptr;
		limits = {};
		callback = std::bind(&ShellStats::all_times_stats, this);
		break;
	    }

	    callback();

	    cout << colors::YELLOW;
	    cout << endl << endl;
	    cout << "--------------------------------------------------------------------------------" << endl;
	    cout << "n -> next    p -> previous    q -> exit" << endl << endl;
	    cout << "Options Menu" << endl;
	    cout << "    d -> Dashboard" << endl;
	    cout << "    y -> Year Stats" << endl;
	    cout << "    a -> All Times Stats" << endl << endl;
	    cout << "Command Actions" << endl;
	    cout << "    :<year> -> jump to the year" << endl;
	    cout << "--------------------------------------------------------------------------------" << endl;
	    cout << colors::RESET;
	}

	c = get_char();

	if (action.empty() && next_prev_ref != nullptr)
	{
	    switch (c)
	    {
	    case 'n':
		if (!limits.has_value())
		{
		    (*next_prev_ref)++;
		}
		else if (*next_prev_ref >= limits.value().first &&
			 *next_prev_ref < limits.value().second)
		{
		    (*next_prev_ref)++;
		}
		else
		{
		    *next_prev_ref = limits.value().first;
		    year = *next_prev_ref == month ? year + 1 : year;
		}
		break;
	    case 'p':
		if (!limits.has_value())
		{
		    (*next_prev_ref)--;
		}
		else if (*next_prev_ref > limits.value().first &&
			 *next_prev_ref <= limits.value().second)
		{
		    (*next_prev_ref)--;
		}
		else
		{
		    *next_prev_ref = limits.value().second;
		    year = *next_prev_ref == month ? year - 1 : year;
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

ShellSteps::ShellSteps(const StepsData &steps_data)
{
    const std::chrono::time_point now{std::chrono::system_clock::now()};
    const std::chrono::year_month_day ymd{std::chrono::floor<std::chrono::days>(now)};

    year = static_cast<int>(ymd.year());
    month = static_cast<unsigned>(ymd.month());
    data = steps_data;
}

void ShellSteps::all_times_stats() const
{
    print_header("STEPS: ALL TIMES YEARLY STATS");

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
	    print_header(std::format("Year {}", current_year));
	    print_steps_stats(all_steps);
	    current_year = idx.year();
	    all_steps = Steps{};
	}
	all_steps += steps;
    }

    print_header(std::format("Year {}", current_year));
    print_steps_stats(all_steps);
}

void ShellSteps::year_stats() const
{
    std::ostringstream oss;
    oss << "STEPS: YEAR DASHBOARD - " << year;
    print_header(oss.str());

    auto itr = std::find_if(
	this->data.steps.cbegin(),
	this->data.steps.cend(),
	[this](const auto& item) { return item.first.year() == this->year; });
    if (itr == this->data.steps.end())
    {
	cout << "There are not data for this date" << endl;
	return;
    }

    Steps all_steps{};
    while (itr != this->data.steps.end() && itr->first.year() == year)
    {
	all_steps += itr->second;
	itr++;
    }

    print_steps_stats(all_steps);
}

void ShellSteps::month_stats() const
{
    std::ostringstream oss;
    oss << "STEPS: MONTH DASHBOARD - " << year << ", " << MONTHS_NAMES[month - 1];
    print_header(oss.str());

    Calendar calendar{year, month};
    auto first_wd_ymd = calendar.get_first_wd_ymd();
    auto last_wd_ymd = calendar.get_last_wd_ymd();

    auto itr = std::find_if(
	this->data.steps.cbegin(),
	this->data.steps.cend(),
	[this, first_wd_ymd](const auto& item) {
	    return item.first.ymd() == first_wd_ymd ||
		(item.first.year() == this->year && item.first.month() == this->month);
	});
    if (itr == this->data.steps.end())
    {
     	cout << "There are not data for this date " << endl;
     	return;
    }

    Steps all_steps{};
    size_t count_days = 0;
    auto weekly_steps = std::map<std::string, Steps>();
    size_t week_number = 1;
    size_t month_day = 1;
    while (itr != this->data.steps.end() && itr->first.ymd() <= last_wd_ymd)
    {
	if (itr->first.month() == month)
	{
	    all_steps += itr->second;
	    ++count_days;
	}

	weekly_steps["Week " + std::to_string(week_number)] += itr->second;
	if (month_day % 7 == 0)
	    ++week_number;

	std::string s1 = std::to_string(itr->second.steps) + " steps";
	std::string s2 = std::to_string(
	    static_cast<int>(std::round(itr->second.distance))) + " m";
	std::string s3 = std::to_string(itr->second.calories) + " kcal";

	calendar.add(itr->first.ymd(), s1);
	calendar.add(itr->first.ymd(), s2);
	calendar.add(itr->first.ymd(), s3);

	++month_day;
	++itr;
    }

    calendar.print();

    cout << endl;
    auto tabular = Tabular();
    for (const auto& [key, value] : weekly_steps)
    {
	tabular.add_header(key);
	tabular.add_values(
	    key,
	    {
		unit(value.steps, "steps"),
		distance(value.distance),
		unit(value.calories, "kcal"),
		unit(value.steps / 7, "steps/day")
	    });
    }
    tabular.print();

    cout << endl;
    print_header("Total steps for month: " + MONTHS_NAMES[month - 1]);
    print_steps_stats(all_steps, count_days);
}

ShellSleep::ShellSleep(const SleepData &sleep_data)
{
    const std::chrono::time_point now{std::chrono::system_clock::now()};
    const std::chrono::year_month_day ymd{std::chrono::floor<std::chrono::days>(now)};

    year = static_cast<int>(ymd.year());
    month = static_cast<unsigned>(ymd.month());
    data = sleep_data;
}

void ShellSleep::all_times_stats() const
{
    print_header("SLEEP: ALL TIMES YEARLY STATS");

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
	    print_header(std::format("Year {}", current_year));
	    print_sleep_stats(swc.sleep, swc.count);
	    current_year = idx.year();
	    swc = SleepWithCount{};
	}
	swc = SleepWithCount{swc.sleep + sleep, swc.count + 1};
    }

    print_header(std::format("Year {}", current_year));
    print_sleep_stats(swc.sleep, swc.count);
}

void ShellSleep::year_stats() const
{
    std::ostringstream oss;
    oss << "SLEEP: YEAR DASHBOARD - " << year;
    print_header(oss.str());

    auto itr = std::find_if(
	this->data.sleep.cbegin(),
	this->data.sleep.cend(),
	[this](const auto& item) { return item.first.year() == this->year; });
    if (itr == this->data.sleep.end())
    {
	cout << "There are not data for this year" << endl;
	return;
    }

    SleepWithCount swc{};
    while (itr != this->data.sleep.end() && itr->first.year() == year)
    {
	swc = SleepWithCount{swc.sleep + itr->second, swc.count + 1};
	itr++;
    }

    print_sleep_stats(swc.sleep, swc.count);
}

void ShellSleep::month_stats() const
{
    std::ostringstream oss;
    oss << "SLEEP: MONTH DASHBOARD - " << year << ", " << MONTHS_NAMES[month - 1];
    print_header(oss.str());

    Calendar calendar{year, month};
    auto first_wd_ymd = calendar.get_first_wd_ymd();
    auto last_wd_ymd = calendar.get_last_wd_ymd();

    auto itr = std::find_if(
	this->data.sleep.cbegin(),
	this->data.sleep.cend(),
	[this, first_wd_ymd](const auto& item) {
	    return item.first.ymd() == first_wd_ymd ||
		(item.first.year() == this->year && item.first.month() == this->month);
	});
    if (itr == this->data.sleep.end())
    {
     	cout << "There are not data for this date " << endl;
     	return;
    }

    SleepWithCount swc{};
    while (itr != this->data.sleep.end() && itr->first.ymd() <= last_wd_ymd)
    {
	if (itr->first.month() == month)
	    swc = SleepWithCount{swc.sleep + itr->second, swc.count + 1};

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

    if (swc.count > 0)
    {
	cout << endl;
	print_header(MONTHS_NAMES[month - 1] + ": average");
	print_sleep_stats(swc.sleep, swc.count);
    }
}

inline std::vector<std::string> activities_stats(const Activity* a)
{
    std::vector<std::string> result{};

    if (a->total_work_time.has_value())
	result.emplace_back(
	    value_formatted("Work Time", time(a->total_work_time.value()), 25));
    if (a->total_elapsed_time.has_value())
	result.emplace_back(
	    value_formatted("Elapsed Time", time(a->total_elapsed_time.value()), 25));
    if (a->total_timer_time.has_value())
	result.emplace_back(
	    value_formatted("Timer Time", time(a->total_timer_time.value()), 25));
    if (a->total_distance.has_value())
	result.emplace_back(value_formatted("Distance", distance(a->total_distance.value()), 25));
    if (a->avg_speed.has_value())
	result.emplace_back(value_formatted("Avg Speed", speed(a->avg_speed.value()), 25));
    if (a->max_speed.has_value())
	result.emplace_back(value_formatted("Max Speed", speed(a->max_speed.value()), 25));
    if (a->total_ascent.has_value())
	result.emplace_back(value_formatted("Ascent", elevation(a->total_ascent.value()), 25));
    if (a->total_descent.has_value())
	result.emplace_back(value_formatted("Descent", elevation(a->total_descent.value()), 25));
    if (a->total_calories.has_value())
	result.emplace_back(value_formatted("Calories", calories(a->total_calories.value()), 25));
    if (a->avg_temperature.has_value())
	result.emplace_back(value_formatted("Avg Temp", temperature(a->avg_temperature.value()), 25));
    if (a->max_temperature.has_value())
	result.emplace_back(value_formatted("Max Temp", temperature(a->max_temperature.value()), 25));
    if (a->min_temperature.has_value())
	result.emplace_back(value_formatted("Min Temp", temperature(a->min_temperature.value()), 25));
    if (a->avg_respiration_rate.has_value())
	result.emplace_back(value_formatted("Avg Resp", value(a->avg_respiration_rate.value()), 25));
    if (a->max_respiration_rate.has_value())
	result.emplace_back(value_formatted("Max Resp", value(a->max_respiration_rate.value()), 25));
    if (a->min_respiration_rate.has_value())
	result.emplace_back(value_formatted("Min Resp", value(a->min_respiration_rate.value()), 25));
    if (a->training_load_peak.has_value())
	result.emplace_back(value_formatted("Load Peak", value(a->training_load_peak.value()), 25));
    if (a->total_training_effect.has_value())
	result.emplace_back(value_formatted("Train Effect", value(a->total_training_effect.value()), 25));
    if (a->total_anaerobic_training_effect.has_value())
	result.emplace_back(
	    value_formatted("Anaerobic Effect", value(a->total_anaerobic_training_effect.value()), 25));
    return result;
}

ShellActivities::ShellActivities(const ActivitiesData& activities_data)
{
    const std::chrono::time_point now{std::chrono::system_clock::now()};
    const std::chrono::year_month_day ymd{std::chrono::floor<std::chrono::days>(now)};

    year = static_cast<int>(ymd.year());
    month = static_cast<unsigned>(ymd.month());
    data = activities_data;
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
	    {
		"Distance",
		aggregated->total_distance.has_value() ? distance(aggregated->total_distance.value()) : "-"
	    },
	    {
		"Work Time",
		aggregated->total_work_time.has_value() ? time(aggregated->total_work_time.value()) : "-"
	    },
	    {
		"Calories",
		aggregated->total_calories.has_value() ? calories(aggregated->total_calories.value()) : "-"
	    }
	});
    summary.print();

    cout << endl;

    auto tabular = Tabular();
    auto sport_stats = SportStats(year, month, this->data.activities);
    std::string header{};
    for (const auto& [sport, s_stats] : sport_stats.get_stats())
    {
	header = sport + " (" + std::to_string(s_stats.get_count()) + ")";
	tabular.add_header(header);
	tabular.add_values(header, activities_stats(s_stats.get_stats().get()));
    }
    tabular.print();
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
	    {
		"Distance",
		aggregated->total_distance.has_value() ? distance(aggregated->total_distance.value()) : "-"
	    },
	    {
		"Work Time",
		aggregated->total_work_time.has_value() ? time(aggregated->total_work_time.value()) : "-"
	    },
	    {
		"Calories",
		aggregated->total_calories.has_value() ? calories(aggregated->total_calories.value()) : "-"
	    }
	});
    summary.print();

    cout << endl;

    auto tabular = Tabular();
    auto sport_stats = SportStats(year, this->data.activities);
    std::string header{};
    for (const auto& [sport, s_stats] : sport_stats.get_stats())
    {
	header = sport + " (" + std::to_string(s_stats.get_count()) + ")";
	tabular.add_header(header);
	tabular.add_values(header, activities_stats(s_stats.get_stats().get()));
    }
    tabular.print();
}

void ShellActivities::all_times_stats() const
{
    print_header("ACTIVITIES STATS");

    const auto stats = AggregatedStats(this->data.activities);
    if (stats.empty())
    {
        cout << "There are not activities to show" << endl;
        return;
    }

    print_aggregated_stats(stats);
}

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
	else if (itr->second->total_work_time.has_value())
	    s = itr->second->sport_profile_name +
		 " (" + time(itr->second->total_work_time.value()) + ")";
	else
	    s = itr->second->sport_profile_name +
		 " (" + time(itr->second->total_elapsed_time.value()) + ")";
	calendar.add(itr->first.ymd(), s);
	itr++;
    }

    calendar.print();
}

} // namespace fitgalgo
