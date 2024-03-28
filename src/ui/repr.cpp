#include <iomanip>
#include <sstream>
#include <chrono>
#include <cmath>
#include <iostream>

#include "repr.h"

namespace fitgalgo
{

template <typename T>
std::string formatted_number(const T& n)
{
    std::string res = std::to_string(n);
    size_t pos = res.find('.');
    short mils_pos{};

    if (pos == std::string::npos)
	mils_pos = res.length() > 3 ? res.length() - 3: 0;
    else
	mils_pos -= 3;

    while (mils_pos > 0)
    {
        res.insert(mils_pos, " ");
        mils_pos = mils_pos > 3 ? mils_pos - 3 : 0;
    }

    size_t decimal_pos = res.find('.');
    if (decimal_pos != std::string::npos && decimal_pos + 3 < res.length())
	res = res.substr(0, decimal_pos + 3);

    return res;
}

std::string time(const float &v)
{
    int h = v / 3600;
    int min = (static_cast<int>(v) % 3600) / 60;
    int sec = (static_cast<int>(v) % 3600) % 60;
    std::string result = "";

    if (h > 0)
	result = formatted_number(h) + "h ";
    if (min > 0)
	result += formatted_number(min) + "min ";
    if (sec > 0)
	result += formatted_number(sec) + "s";

    return result;
}

std::string date(const std::chrono::year_month_day& v)
{
    std::chrono::system_clock::time_point tp = std::chrono::sys_days{v};
    std::time_t t = std::chrono::system_clock::to_time_t(tp);
    char mbstr[100];
    if (std::strftime(mbstr, sizeof(mbstr), "%A, %d %B %Y", std::localtime(&t)))
        return mbstr;

    std::ostringstream oss;
    oss << v;
    return oss.str();
}

std::string distance(const float &v)
{
    if (v >= 1000)
	return formatted_number(v / 1000) + " km";
    else
	return formatted_number(v) + " m";
}

std::string speed(const float& v)
{
    return formatted_number(v * 3.6) + " km/h";
}

std::string pace(const float& speed_mps)
{
    float kmph = speed_mps * 3.6;
    float kmpmin = 60 / kmph;
    float min;
    float decimal = std::modf(kmpmin, &min);
    float seconds = decimal * 60;

    std::stringstream ss;
    ss << std::setw(2) << std::setfill('0') << static_cast<int>(min)
       << ':'
       << std::setw(2) << std::setfill('0') << static_cast<int>(seconds)
       << " min/km";
    return ss.str();
}

std::string heart_rate(const float &v)
{
    return formatted_number(static_cast<int>(v)) + " bpm";
}

std::string elevation(const float& v)
{
    return formatted_number(static_cast<int>(v)) + " m";
}

std::string calories(const float& v)
{
    return formatted_number(static_cast<int>(v)) + " kcal";
}

std::string temperature(const float& v)
{
    return formatted_number(v) + " ºC";
}

std::string value(const float& v)
{
    return formatted_number(v);
}

std::string ivalue(const int &v)
{
    return formatted_number(v);
}

std::string unit(const float& v)
{
    return formatted_number(v);
}

std::string unit(const float& v, const std::string& u)
{
    return formatted_number(v) + " " + u;
}

std::string unit(const int& v)
{
    return formatted_number(v);
}

std::string unit(const int& v, const std::string& u)
{
    return formatted_number(v) + " " + u;
}

} // namespace fitgalgo
