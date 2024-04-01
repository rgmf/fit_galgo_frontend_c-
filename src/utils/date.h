#ifndef ES_RGMF_UTILS_DATE_H
#define ES_RGMF_UTILS_DATE_H 1

#include <chrono>

using chrono_ymd = std::chrono::year_month_day;
using chrono_year = std::chrono::year;
using chrono_month = std::chrono::month;
using chrono_day = std::chrono::day;

namespace fitgalgo
{

inline bool is_valid_date(const short& y, const short& m, const short& d)
{
    if (y < 0 || m < 1 || m > 12 || d < 1 || d > 31)
        return false;

    if ((m == 4 || m == 6 || m == 9 || m == 11) && d > 30)
        return false;

    if (m == 2) {
        bool leap_y = (y % 4 == 0 && y % 100 != 0) || (y % 400 == 0);
        if ((leap_y && d > 29) || (!leap_y && d > 28))
            return false;
    }

    return true;
}

inline bool is_valid_time(const short& h, const short& m, const short& s)
{
    return h >= 0 && h <= 24 && m >= 0 && m <= 59 && s >= 0 && s <= 59;
}

inline bool is_valid_zoneinfo(const short& h, const short &m)
{
    return h >= -23 && h <= 23 && m >= 0 && m <= 59;
}

inline chrono_ymd today_ymd()
{
    const std::chrono::time_point now{std::chrono::system_clock::now()};
    const chrono_ymd ymd{std::chrono::floor<std::chrono::days>(now)};
    return ymd;
}

inline ushort last_month_day(const int& year, const ushort& month)
{
    auto first_day_of_next_month = chrono_year(year) /
	chrono_month(month + 1) /
	chrono_day(1);
    auto last_day_of_this_month = chrono_ymd(std::chrono::sys_days(first_day_of_next_month) -
 std::chrono::days(1));
    return unsigned(last_day_of_this_month.day());
}

inline chrono_ymd from_isodate_to_ymd(const std::string& iso_date)
{
    std::stringstream ss(iso_date);
    std::string token;
    std::getline(ss, token, '/');
    int year = std::stoi(token);
    std::getline(ss, token, '/');
    unsigned int month = std::stoi(token);
    std::getline(ss, token);
    unsigned int day = std::stoi(token);
    return chrono_ymd{chrono_year{year}, chrono_month{month}, chrono_day{day}};
}

} // namespace fitgalgo

#endif // ES_RGMF_UTILS_DATE_H
