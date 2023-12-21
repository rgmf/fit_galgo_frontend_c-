#ifndef _ES_RGMF_CORE_API_H
#define _ES_RGMF_CORE_API_H 1

#include <filesystem>
#include <string>
#include <map>
#include <vector>

#include "httplib/httplib.h"
#include "rapidjson/document.h"

namespace fitgalgo
{

class Data
{
public:
    virtual ~Data() = default;
    virtual bool load(rapidjson::Document& document) = 0;
};

class LoginData : public Data
{
public:
    std::string access_token;
    std::string token_type;

    bool load(rapidjson::Document& document) override;
};

class UploadedFileData : public Data
{
public:
    std::string id;
    std::string file_path;
    bool accepted;
    std::vector<std::string> errors;
    std::string zip_file_path;

    bool load(rapidjson::Document& document) override;
};

struct Steps
{
    std::string datetime_utc{};
    std::string datetime_local{};
    int steps{};
    float distance{};
    int calories{};

    Steps& operator+=(const Steps& rhs)
    {
	this->datetime_utc = rhs.datetime_utc;
	this->datetime_local = rhs.datetime_local;
	this->steps += rhs.steps;
	this->distance += rhs.distance;
	this->calories += rhs.calories;
        return *this;
    }

    friend Steps operator+(Steps lhs, const Steps& rhs)
    {
	lhs.datetime_utc = rhs.datetime_utc;
	lhs.datetime_local = rhs.datetime_local;
        lhs.steps += rhs.steps;
	lhs.distance += rhs.distance;
	lhs.calories += rhs.calories;
        return lhs;
    }
};

class DateIdx
{
private:
    std::string date{};

    inline static bool is_long_date(const std::string& value)
    {
	if (value.length() < 10)
	{
	    return false;
	}
	std::string date = value.substr(0, 10);
	std::string format{"%Y-%m-%d"};
	std::tm tm_struct = {};
	std::istringstream ss(date);
	ss >> std::get_time(&tm_struct, format.c_str());
	return !ss.fail();
    }

    inline static bool is_short_date(const std::string& value)
    {
	if (value.length() < 8)
	{
	    return false;
	}
	std::string date = value.substr(0, 8);
	std::string format{"%Y%m%d"};
	std::tm tm_struct = {};
	std::istringstream ss(date);
	ss >> std::get_time(&tm_struct, format.c_str());
	return !ss.fail();
    }

public:
    DateIdx(const std::string& value)
    {
	if (is_long_date(value))
	{
	    this->date = value.substr(0, 10);
	}
	else if (is_short_date(value))
	{
	    this->date = value.substr(0, 4) + "-" +
		    value.substr(4, 2) + "-" + value.substr(6, 2);
	}
	else
	{
	    this->date = {};
	}
    }

    const std::string& value() const { return this->date; }

    bool is_valid() const { return !this->date.empty(); }

    static bool is_valid(const std::string& value)
    {
	return is_long_date(value) || is_short_date(value);
    }

    friend bool operator<(const DateIdx& l, const DateIdx& r)
    {
	return l.date < r.date;
    }
};

class StepsData : public Data
{
public:
    std::map<DateIdx, fitgalgo::Steps> steps{};
    std::vector<std::string> errors{};
    bool load(rapidjson::Document& document) override;
};

struct SleepAssessment
{
    float combined_awake_score{};
    float awake_time_score{};
    float awakenings_count_score{};
    float deep_sleep_score{};
    float sleep_duration_score{};
    float light_sleep_score{};
    float overall_sleep_score{};
    float sleep_quality_score{};
    float sleep_recovery_score{};
    float rem_sleep_score{};
    float sleep_restlessness_score{};
    float awakenings_count{};
    float interruptions_score{};
    float average_stress_during_sleep{};

    SleepAssessment& operator+=(const SleepAssessment& rhs)
    {
	this->combined_awake_score += rhs.combined_awake_score;
	this->awake_time_score += rhs.awake_time_score;	
	this->awakenings_count_score += rhs.awakenings_count_score;	
	this->deep_sleep_score += rhs.deep_sleep_score;	
	this->sleep_duration_score += rhs.sleep_duration_score;	
	this->light_sleep_score += rhs.light_sleep_score;	
	this->overall_sleep_score += rhs.overall_sleep_score;	
	this->sleep_quality_score += rhs.sleep_quality_score;	
	this->sleep_recovery_score += rhs.sleep_recovery_score;	
	this->rem_sleep_score += rhs.rem_sleep_score;	
	this->sleep_restlessness_score += rhs.sleep_restlessness_score;
	this->awakenings_count += rhs.awakenings_count;
	this->interruptions_score += rhs.interruptions_score;	
	this->average_stress_during_sleep += rhs.average_stress_during_sleep;

	return *this;
    }

    friend SleepAssessment operator+(SleepAssessment lhs, const SleepAssessment& rhs)
    {
	lhs.combined_awake_score += rhs.combined_awake_score;	
	lhs.awake_time_score += rhs.awake_time_score;	
	lhs.awakenings_count_score += rhs.awakenings_count_score;	
	lhs.deep_sleep_score += rhs.deep_sleep_score;	
	lhs.sleep_duration_score += rhs.sleep_duration_score;	
	lhs.light_sleep_score += rhs.light_sleep_score;
	lhs.overall_sleep_score += rhs.overall_sleep_score;	
	lhs.sleep_quality_score += rhs.sleep_quality_score;	
	lhs.sleep_recovery_score += rhs.sleep_recovery_score;	
	lhs.rem_sleep_score += rhs.rem_sleep_score;	
	lhs.sleep_restlessness_score += rhs.sleep_restlessness_score;	
	lhs.awakenings_count += rhs.awakenings_count;	
	lhs.interruptions_score += rhs.interruptions_score;	
	lhs.average_stress_during_sleep += rhs.average_stress_during_sleep;

	return lhs;
    }
};

struct SleepLevel
{
    std::string datetime_utc{};
    std::string level{};
};

struct Sleep
{
    std::string zone_info{};
    fitgalgo::SleepAssessment assessment{};
    std::vector<fitgalgo::SleepLevel> levels{};
    std::vector<std::string> dates{};

    const std::string datetime_local() const
    {
	std::cout << "datetime_local: ";
	for (const auto& s : dates)
	{
	    std::cout << s << std::endl;
	    if (s.length() >= 8)
	    {
		return s.substr(0, 4) + "-" + s.substr(4, 2) + "-" + s.substr(6, 2);
	    }
	}
	return "";
    }

    Sleep& operator+=(const Sleep& rhs)
    {
	this->zone_info = rhs.zone_info;
	this->assessment += rhs.assessment;
	this->levels.insert(this->levels.end(), rhs.levels.begin(), rhs.levels.end());
	this->dates.insert(this->dates.end(), rhs.dates.begin(), rhs.dates.end());
        return *this;
    }

    friend Sleep operator+(Sleep lhs, const Sleep& rhs)
    {
	lhs.zone_info = rhs.zone_info;
	lhs.assessment += rhs.assessment;
	lhs.levels.insert(lhs.levels.end(), rhs.levels.begin(), rhs.levels.end());
	lhs.dates.insert(lhs.dates.end(), rhs.dates.begin(), rhs.dates.end());
        return lhs;
    }
};

struct SleepWithCount
{
    Sleep sleep{};
    size_t count{};
};

class SleepData : public Data
{
public:
    std::map<DateIdx, fitgalgo::Sleep> sleep{};
    std::vector<std::string> errors{};
    bool load(rapidjson::Document& document) override;
};

enum class ErrorType {
    Success = 0,
    NotLoaded,
    Unknown,
    NotResponse,
    NotData,
    Http100,
    Http300,
    Http400,
    Http401,
    Http500
};

class Error
{
private:
    ErrorType error = ErrorType::NotLoaded;
    httplib::Error httplib_error = httplib::Error::Success;

public:
    Error() {}
    Error(const ErrorType error, const httplib::Error httplib_error = httplib::Error::Success)
	: error(error), httplib_error(httplib_error) {}

    bool has_error() const;
    std::string error_to_string() const;
};

template <typename T>
class Result
{
private:
    Error error{};
    int status{};
    std::unique_ptr<T> data{};

public:
    void load(const httplib::Result& response);
    void load(const T& newData);
    bool is_valid() const { return !this->error.has_error(); }
    const Error get_error() const { return error; }
    const T& get_data() { return *this->data; }
};

/**
 * Class for getting data from Fit Galgo API.
 *
 * It also manages login, logout and the token.
 */
class Connection
{
private:
    std::string host;
    int port;
    std::string token{};

    const fitgalgo::Result<fitgalgo::UploadedFileData> do_post_for_file(
	httplib::Client& client, const std::filesystem::path& file_path) const;

public:
    Connection(const std::string host, const int port)
	: host(host), port(port) {}
    const fitgalgo::Result<fitgalgo::LoginData> login(
	const std::string& username, const std::string& password);
    void logout();
    bool has_token() const;
    const std::vector<fitgalgo::Result<fitgalgo::UploadedFileData>> post_file(
	std::filesystem::path& path) const;
    const fitgalgo::Result<fitgalgo::StepsData> get_steps() const;
    const fitgalgo::Result<fitgalgo::SleepData> get_sleep() const;
};

} // namespace fitgalgo

#endif // _ES_RGMF_CORE_API_H
