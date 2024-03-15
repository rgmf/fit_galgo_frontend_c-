#ifndef _ES_RGMF_UTILS_DATE_H
#define _ES_RGMF_UTILS_DATE_H 1

#include <chrono>

namespace fitgalgo
{

inline std::chrono::year_month_day today_ymd()
{
    const std::chrono::time_point now{std::chrono::system_clock::now()};
    const std::chrono::year_month_day ymd{std::chrono::floor<std::chrono::days>(now)};
    return ymd;
}

inline ushort last_month_day(const int& year, const ushort& month)
{
    auto first_day_of_next_month = std::chrono::year(year) /
	std::chrono::month(month + 1) /
	std::chrono::day(1);
    auto last_day_of_this_month = std::chrono::year_month_day(
	std::chrono::sys_days(first_day_of_next_month) - std::chrono::days(1));
    return unsigned(last_day_of_this_month.day());
}

inline std::chrono::year_month_day from_isodate_to_ymd(const std::string& iso_date)
{
    std::stringstream ss(iso_date);
    std::string token;
    std::getline(ss, token, '/');
    int year = std::stoi(token);
    std::getline(ss, token, '/');
    unsigned int month = std::stoi(token);
    std::getline(ss, token);
    unsigned int day = std::stoi(token);
    return std::chrono::year_month_day{std::chrono::year{year}, std::chrono::month{month}, std::chrono::day{day}};
}

} // namespace fitgalgo

#endif // _ES_RGMF_UTILS_DATE_H
