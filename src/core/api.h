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
    std::string datetime_utc;
    std::string datetime_local;
    int steps;
    float distance;
    int calories;

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

class StepsData : public Data
{
public:
    std::map<std::string, fitgalgo::Steps> steps{};
    std::vector<std::string> errors{};
    bool load(rapidjson::Document& document) override;
};

struct SleepAssessment
{
    int combined_awake_score;
    int awake_time_score;
    int awakenings_count_score;
    int deep_sleep_score;
    int sleep_duration_score;
    int light_sleep_score;
    int overall_sleep_score;
    int sleep_quality_score;
    int sleep_recovery_score;
    int rem_sleep_score;
    int sleep_restlessness_score;
    int awakenings_count;
    int interruptions_score;
    float average_stress_during_sleep;
};

struct SleepLevel
{
    std::string datetime_utc;
    std::string level;
};

struct Sleep
{
    std::string zone_info;
    fitgalgo::SleepAssessment assessment;
    std::vector<fitgalgo::SleepLevel> levels;
    std::vector<std::string> dates;
};

class SleepData : public Data
{
public:
    std::map<std::string, fitgalgo::Sleep> sleep{};
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
