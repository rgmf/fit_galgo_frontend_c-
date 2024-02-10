#include <cstddef>
#include <iostream>
#include <chrono>
#include <algorithm>
#include <sstream>
#include <string>

#include "calendar.h"
#include "colors.h"

using std::cout;
using std::endl;

namespace fitgalgo
{

Calendar::Calendar(const ushort year, const ushort month)
{
    this->year = year;
    this->month = month;
    
    this->first_ymd = std::chrono::year_month_day(
	std::chrono::year(year), std::chrono::month(month), std::chrono::day(1));
    this->last_ymd = std::chrono::year_month_day_last(
	this->first_ymd.year(), this->first_ymd.month() / std::chrono::last);

    auto sys_days = std::chrono::sys_days(this->first_ymd);
    ushort days_to_monday = std::chrono::weekday(this->first_ymd).iso_encoding() - 1;
    this->first_wd_ymd = std::chrono::year_month_day(sys_days - std::chrono::days(days_to_monday));

    sys_days = std::chrono::sys_days(this->last_ymd);
    ushort days_to_sunday = 7 - std::chrono::weekday(this->last_ymd).iso_encoding();
    this->last_wd_ymd = std::chrono::year_month_day(sys_days + std::chrono::days(days_to_sunday));

    auto ymd = this->first_wd_ymd;
    while (ymd <= this->last_wd_ymd)
    {
	for (size_t col = 0; col < NUMBER_OF_WEEKDAYS; col++)
	{
	    this->cells.emplace_back(
		Cell(std::chrono::year_month_day(ymd.year(), ymd.month(), ymd.day())));
	    sys_days = std::chrono::sys_days(ymd);
	    ymd = std::chrono::year_month_day(sys_days + std::chrono::days(1));
	}
    }
}

void Calendar::add(const std::chrono::year_month_day& ymd, const std::string& value)
{
    const Cell cell(ymd);
    const auto& itr = std::find(this->cells.begin(), this->cells.end(), cell);
    if (itr != this->cells.end())
    {
	itr->append(value);
    }
}

inline void Calendar::print_new_line() const
{
    cout << endl;
    for (ushort i = 0; i < DAYS_ABBR.size(); i++)
	for (ushort j = 0; j < CELL_MAX_WIDTH; j++)
	    cout << '-';
    cout << endl;
}

inline void Calendar::print_header() const
{
    std::ostringstream header;
    header << this->year << ", " << MONTHS_NAMES[this->month - 1];
    const size_t total_weight = CELL_MAX_WIDTH * NUMBER_OF_WEEKDAYS;
    const size_t header_weight = header.str().length();

    for (size_t i = 0; i < (total_weight - header_weight) / 2; i++)
	cout << ' ';

    cout << colors::BOLD << header.str() << endl;
    for (size_t i = 0; i < total_weight; i++)
	cout << "-";
    cout << endl;

    for (const auto& a : DAYS_ABBR)
    {
	std::string final_str = a.length() >= CELL_MAX_WIDTH ? a.substr(0, CELL_MAX_WIDTH - 1) : a;

	cout << final_str;
	size_t extra_spaces = CELL_MAX_WIDTH - final_str.length();
	for (size_t i = 0; i < extra_spaces; i++)
	    cout << ' ';
    }
    this->print_new_line();
    cout << colors::RESET;
}

void Calendar::print() const
{
    this->print_header();

    bool has_more = false;
    size_t i = 0;
    auto first_wd_itr = this->cells.begin();
    auto itr = first_wd_itr;
    while (first_wd_itr < this->cells.end())
    {
        for (size_t j = 0; j < NUMBER_OF_WEEKDAYS && itr != this->cells.end(); j++, itr++)
	{
	    itr->print(i, this->month);

	    if (itr->size() > i + 1)
		has_more = true;
	}

	if (has_more)
	{
	    has_more = false;
	    i++;
	    itr = first_wd_itr;
	    cout << endl;
	}
	else
	{
	    has_more = true;
	    i = 0;
	    first_wd_itr = itr;
	    print_new_line();
	}
    }
}

inline void Cell::print(const ushort& i, const ushort& month) const
{
    std::string s = i < this->values.size() ? this->values[i] : "";
    std::string final_str = s.length() >= CELL_MAX_WIDTH ? s.substr(0, CELL_MAX_WIDTH - 1) : s;

    if (static_cast<unsigned>(this->ymd.month()) != month)
	cout << "\033[0;37m";
    cout << final_str;
    if (static_cast<unsigned>(this->ymd.month()) != month)
	cout << "\033[0m";
    size_t extra_spaces = CELL_MAX_WIDTH - final_str.length();
    for (size_t i = 0; i < extra_spaces; i++)
	cout << ' ';
}

const std::string& Cell::get(const ushort& i) const
{
    if (i < this->values.size())
	return this->values.at(i);
    else
    {
	static const std::string empty = "";
	return empty;
    }
}

void Cell::append(const std::string& value)
{
    this->values.emplace_back(value);
}

}
