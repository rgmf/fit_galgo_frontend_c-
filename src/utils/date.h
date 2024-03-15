#ifndef _ES_RGMF_UTILS_DATE_H
#define _ES_RGMF_UTILS_DATE_H 1

#include <chrono>

namespace fitgalgo
{

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
