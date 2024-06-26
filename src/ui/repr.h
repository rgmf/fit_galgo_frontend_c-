#ifndef _ES_RGMF_UI_REPR_H
#define _ES_RGMF_UI_REPR_H 1

#include <chrono>
#include <string>

namespace fitgalgo
{    

std::string time(const float& v); 
std::string date(const std::chrono::year_month_day& v); 
std::string distance(const float& v);
std::string speed(const float& v);
std::string pace(const float& speed_mps);
std::string heart_rate(const float& v);
std::string elevation(const float& v); 
std::string calories(const float& v); 
std::string temperature(const float& v);
std::string value(const float &v);
std::string ivalue(const int &v);
std::string unit(const float& v);
std::string unit(const float& v, const std::string& u);
std::string unit(const int& v);
std::string unit(const int& v, const std::string& u);

} // namespace fitgalgo

#endif // _ES_RGMF_UI_REPR_H
