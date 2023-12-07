#include <chrono>
#include <string>
#include <ctime>

#include "stats.h"
#include "api.h"

namespace fitgalgo
{

std::chrono::system_clock::time_point parse_dt(
    const std::string& dt, const std::string& format)
{
    tm tm_struct = {};
    std::istringstream ss(dt);
    ss >> std::get_time(&tm_struct, format.c_str());
    return std::chrono::system_clock::from_time_t(std::mktime(&tm_struct));
}

template <typename T>
Stats<T>::Stats(const std::map<std::string, T>& data)
{
    std::chrono::system_clock::time_point time_point;
    time_t date;
    tm* dateinfo;
    int year, month, week, day;
    char buffer[3];

    for (const auto& [dt, object] : data)
    {
	time_point = parse_dt(dt, "%Y-%m-%d");
	date = std::chrono::system_clock::to_time_t(time_point);
	dateinfo = localtime(&date);

	year = dateinfo->tm_year + 1900;
	month = dateinfo->tm_mon;
	strftime(buffer, sizeof(buffer), "%W", dateinfo);
	try
	{
	    week = std::stoi(buffer);
	}
	catch (const std::invalid_argument& e)
	{
	    week = 0;
	}
	day = dateinfo->tm_wday;

	if (!this->years.contains(year))
	    this->years[year] = YearStats<T>();

	if (!this->years[year].months.contains(month))
	    this->years[year].months[month] = MonthStats<T>();

	if (!this->years[year].months[month].weeks.contains(week))
	    this->years[year].months[month].weeks[week] = WeekStats<T>();

	this->years[year].months[month].weeks[week].days[day] += object;
    }
}

template class Stats<Steps>;

} //namespace fitgalgo
