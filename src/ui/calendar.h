#ifndef _ES_RGMF_UI_CALENDAR_PRINTER_H
#define _ES_RGMF_UI_CALENDAR_PRINTER_H 1

#include <array>
#include <map>
#include <string>
#include <chrono>

namespace fitgalgo
{

const size_t CELL_MAX_WIDTH = 20;

const size_t NUMBER_OF_WEEKDAYS = 7;

const std::string MONTHS_NAMES[] = { "January", "February", "March", "April",
				     "May",	"June", "July", "August", "September",
				     "October", "November", "December" };
const std::string ISO_DAYS_NAMES[] = { "Monday", "Tuesday", "Wednesday",
				       "Thursday", "Friday", "Saturday", "Sunday" };
const std::array<std::string, NUMBER_OF_WEEKDAYS> DAYS_ABBR{
    "Mon", "Tue", "Wed", "Thr", "Fri", "Sat", "Sun"};

class Cell
{
private:
    std::chrono::year_month_day ymd;
    std::vector<std::string> values;

public:
    Cell(const std::chrono::year_month_day &ymd) : ymd(ymd), values() {}
    void append(const std::string &value);
    bool operator==(const Cell& rhs) { return this->ymd == rhs.ymd; }
    const std::string& get(const ushort& i) const;
    size_t size() const { return this->values.size(); }
    inline void print(const ushort& i, const ushort& month) const;
};

class Calendar
{
private:
    ushort year;
    ushort month;
    
    std::chrono::year_month_day first_ymd;
    std::chrono::year_month_day last_ymd;

    std::chrono::year_month_day first_wd_ymd;
    std::chrono::year_month_day last_wd_ymd;

    std::vector<Cell> cells;

    inline void print_header() const;
    inline void print_new_line() const;

public:
    Calendar(const ushort year, const ushort month);
    std::chrono::year_month_day get_first_wd_ymd() const { return this->first_wd_ymd; }
    std::chrono::year_month_day get_last_wd_ymd() const { return this->last_wd_ymd; }
    void add(const std::chrono::year_month_day& ymd, const std::string& value);
    void print() const;
};

}

#endif // _ES_RGMF_UI_CALENDAR_PRINTER_H
