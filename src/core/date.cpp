#include <regex>

#include "date.h"

namespace fitgalgo
{

namespace date
{

inline bool is_valid_date(const date_t& d)
{
    if (d.year < 0 || d.month < 1 || d.month > 12 || d.day < 1 || d.day > 31)
        return false;

    if ((d.month == 4 || d.month == 6 || d.month == 9 || d.month == 11) && d.day > 30)
        return false;

    if (d.month == 2) {
        bool leap_y = (d.year % 4 == 0 && d.year % 100 != 0) || (d.year % 400 == 0);
        if ((leap_y && d.day > 29) || (!leap_y && d.day > 28))
            return false;
    }

    return true;
}

inline bool is_valid_time(const time_t& t)
{
    return t.hour >= 0 && t.hour <= 23 && t.minutes <= 59 && t.seconds <= 59;
}

inline bool is_valid_zoneinfo(const zone_info_t& zi)
{
    return zi.hours >= -23 && zi.hours <= 23 && zi.minutes <= 59;
}

inline chrono_ymd today_ymd()
{
    const std::chrono::time_point now{std::chrono::system_clock::now()};
    const chrono_ymd ymd{std::chrono::floor<chrono_days>(now)};
    return ymd;
}

inline ushort last_month_day(const int& year, const ushort& month)
{
    auto first_day_of_next_month = chrono_year(year) /
	chrono_month(month + 1) /
	chrono_day(1);
    auto last_day_of_this_month = chrono_ymd(chrono_sys_days(first_day_of_next_month) -
					     chrono_days(1));
    return unsigned(last_day_of_this_month.day());
}

inline chrono_ymd from_isodate_to_ymd(const string& iso_date)
{
    std::stringstream ss(iso_date);
    string token;
    std::getline(ss, token, '/');
    int year = std::stoi(token);
    std::getline(ss, token, '/');
    unsigned int month = std::stoi(token);
    std::getline(ss, token);
    unsigned int day = std::stoi(token);
    return chrono_ymd{chrono_year{year}, chrono_month{month}, chrono_day{day}};
}

iso_date::iso_date(const string& v) : value_{v}, ymd_{}, date_type_{date_type_e::INVALID}
{
    std::regex iso8601_date{"^\\d{4}-\\d{2}-\\d{2}$"};
    std::regex iso8601_datetime{"^\\d{4}-\\d{2}-\\d{2}T\\d{2}:\\d{2}:\\d{2}(\\.\\d{3})?$"};
    std::regex iso8601_datetime_zi{"^\\d{4}-\\d{2}-\\d{2}T\\d{2}:\\d{2}:\\d{2}(\\.\\d{3})?(Z|[+-]\\d{2}:\\d{2})$"};

    if (std::regex_match(v, iso8601_date))
    {
	date_ = {
	    .year=(short)(std::atoi(v.substr(0, 4).c_str())),
	    .month=(ushort)(std::atoi(v.substr(5, 2).c_str())),
	    .day=(ushort)(std::atoi(v.substr(8, 2).c_str()))
	};
	if (is_valid_date(date_))
	{
	    ymd_ = chrono_ymd{chrono_year{date_.year},chrono_month{(unsigned)date_.month},
		chrono_day{(unsigned)date_.day}};
	    date_type_ = date_type_e::DATE;
	}
    }
    else if (std::regex_match(v, iso8601_datetime))
    {
	date_ = {
	    .year=(short)(std::atoi(v.substr(0, 4).c_str())),
	    .month=(ushort)(std::atoi(v.substr(5, 2).c_str())),
	    .day=(ushort)(std::atoi(v.substr(8, 2).c_str()))
	};
	time_ = {
	    .hour=(short)std::atoi(v.substr(11, 2).c_str()),
	    .minutes=(ushort)std::atoi(v.substr(14, 2).c_str()),
	    .seconds=(ushort)std::atoi(v.substr(17, 2).c_str()),
	    .millis=(ushort)(v.find('.') != string::npos ? std::atoi(v.substr(20, 3).c_str()) : 0)
	};
	if (is_valid_date(date_) && is_valid_time(time_))
	{
	    ymd_ = chrono_ymd{chrono_year{date_.year},chrono_month{(unsigned)date_.month},
		chrono_day{(unsigned)date_.day}};
	    date_type_ = date_type_e::DATETIME;
	}
    }
    else if (std::regex_match(v, iso8601_datetime_zi))
    {
	date_ = {
	    .year=(short)(std::atoi(v.substr(0, 4).c_str())),
	    .month=(ushort)(std::atoi(v.substr(5, 2).c_str())),
	    .day=(ushort)(std::atoi(v.substr(8, 2).c_str()))
	};
	time_ = {
	    .hour=(short)std::atoi(v.substr(11, 2).c_str()),
	    .minutes=(ushort)std::atoi(v.substr(14, 2).c_str()),
	    .seconds=(ushort)std::atoi(v.substr(17, 2).c_str()),
	    .millis=(ushort)(v.find('.') != string::npos ? std::atoi(v.substr(20, 3).c_str()) : 0)
	};
	size_t p = v.find('Z');
	zone_info_ = {
	    .hours=(short)(p == string::npos ? std::atoi(v.substr(v.size() - 6, 3).c_str()) : 0),
	    .minutes=(ushort)(p == string::npos ? std::atoi(v.substr(v.size() - 2, 2).c_str()) : 0)
	};
	if (is_valid_date(date_) && is_valid_time(time_) && is_valid_zoneinfo(zone_info_))
	{
	    ymd_ = chrono_ymd{chrono_year{date_.year},chrono_month{(unsigned)date_.month},
		chrono_day{(unsigned)date_.day}};
	    date_type_ = date_type_e::DATETIME;
	}
    }
}

const date_type_e& iso_date::date_type() const
{
    return date_type_;
}

const string& iso_date::value() const
{
    return value_;
}

const chrono_ymd& iso_date::ymd() const
{
    return ymd_;
}

const date_t& iso_date::date() const
{
    return date_;
}

const time_t& iso_date::time() const
{
    return time_;
}

const zone_info_t& iso_date::zone_info() const
{
    return zone_info_;
}

iso_date& iso_date::operator--()
{
    ymd_ = chrono_ymd(chrono_sys_days(ymd_) - chrono_days(1));

    date_.year = static_cast<int>(ymd_.year());
    date_.month = static_cast<unsigned>(ymd_.month());
    date_.day = static_cast<unsigned>(ymd_.day());

    std::ostringstream oss;
    oss << std::setw(4) << std::setfill('0') << date_.year
        << '-' << std::setw(2) << std::setfill('0') << date_.month
        << '-' << std::setw(2) << std::setfill('0') << date_.day;
    if (date_type_ == date_type_e::DATETIME)
	value_ = oss.str() + value_.substr(10, value_.length() - 10);
    else
	value_ = oss.str();

    return *this;
}

iso_date& iso_date::operator++()
{
    ymd_ = chrono_ymd(chrono_sys_days(ymd_) + chrono_days(1));

    date_.year = static_cast<int>(ymd_.year());
    date_.month = static_cast<unsigned>(ymd_.month());
    date_.day = static_cast<unsigned>(ymd_.day());

    std::ostringstream oss;
    oss << std::setw(4) << std::setfill('0') << date_.year
        << '-' << std::setw(2) << std::setfill('0') << date_.month
        << '-' << std::setw(2) << std::setfill('0') << date_.day;
    if (date_type_ == date_type_e::DATETIME)
	value_ = oss.str() + value_.substr(10, value_.length() - 10);
    else
	value_ = oss.str();

    return *this;
}

iso_date iso_date::utc() const
{
    auto copy{*this};

    if (copy.zone_info_.hours != 0 || copy.zone_info_.minutes != 0)
    {
	short min = copy.time_.minutes;
	min -= ((copy.zone_info_.hours / abs(copy.zone_info_.hours)) * copy.zone_info_.minutes);
	if (min < 0)
	{
	    copy.time_.minutes = 60 + min;
	    --copy.time_.hour;
	}
	else if (min >= 60)
	{
	    copy.time_.minutes = min - 60;
	    ++copy.time_.hour;
	}
	else
	{
	    copy.time_.minutes = min;
	}

	copy.time_.hour -= copy.zone_info_.hours;
	if (copy.time_.hour < 0)
	{
	    copy.time_.hour = 24 + copy.time_.hour;
	    --copy;
	}
	else if (copy.time_.hour >= 24)
	{
	    copy.time_.hour = copy.time_.hour - 24;
	    ++copy;
	}
    }

    std::ostringstream oss;
    oss << std::setw(4) << std::setfill('0') << copy.date_.year
        << '-' << std::setw(2) << std::setfill('0') << copy.date_.month
        << '-' << std::setw(2) << std::setfill('0') << copy.date_.day
        << 'T' << std::setw(2) << std::setfill('0') << copy.time_.hour
        << ':' << std::setw(2) << std::setfill('0') << copy.time_.minutes
        << ':' << std::setw(2) << std::setfill('0') << copy.time_.seconds;
    if (copy.time_.millis > 0)
	oss << '.' << std::setw(3) << std::setfill('0') << copy.time_.millis;
    oss << 'Z';
    copy.value_ = oss.str();

    return copy;
}

bool operator<(const iso_date& l, const iso_date& r)
{
    return l.utc().value_ < r.utc().value_;
}

}

}
