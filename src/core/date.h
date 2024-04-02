#ifndef ES_RGMF_CORE_DATE_H
#define ES_RGMF_CORE_DATE_H 1

#include <string>
#include <chrono>

using string = std::string;

using chrono_ymd = std::chrono::year_month_day;
using chrono_year = std::chrono::year;
using chrono_month = std::chrono::month;
using chrono_day = std::chrono::day;
using chrono_days = std::chrono::days;
using chrono_sys_days = std::chrono::sys_days;

namespace fitgalgo
{

namespace date
{

enum date_type_e
{
    INVALID = -1,
    DATE,
    DATETIME
};

struct date_t
{
    short year{};
    ushort month{};
    ushort day{};
};

struct time_t
{
    short hour{};
    ushort minutes{};
    ushort seconds{};
    ushort millis{};
};

struct zone_info_t
{
    short hours{};
    ushort minutes{};
};

inline bool is_valid_date(const date_t& d);
inline bool is_valid_time(const time_t& t);
inline bool is_valid_zoneinfo(const zone_info_t& zi);
inline chrono_ymd today_ymd();
inline ushort last_month_day(const int& year, const ushort& month);
inline chrono_ymd from_isodate_to_ymd(const string& iso_date);

class iso_date
{
private:
    string value_;
    chrono_ymd ymd_;
    date_t date_;
    time_t time_;
    zone_info_t zone_info_;
    date_type_e date_type_;

public:
    iso_date() = delete;
    explicit iso_date(const string& v);

    const date_type_e& date_type() const;

    const string& value() const;
    const chrono_ymd& ymd() const;
    const date_t& date() const;
    const time_t& time() const;
    const zone_info_t& zone_info() const;

    iso_date iso_utc() const;

    iso_date& operator--();
    iso_date& operator++();

    friend bool operator<(const iso_date& l, const iso_date& r);
};

}

}

#endif // ES_RGMF_CORE_DATE_H
