#ifndef _ES_RGMF_FITGALGO_H
#define _ES_RGMF_FITGALGO_H 1

#include <filesystem>
#include <string>
#include <map>
#include <vector>

#include "httplib/httplib.h"
#include "rapidjson/document.h"

namespace fitgalgo
{
    class Data {
    public:
	virtual ~Data() = default;
	virtual bool load(rapidjson::Document& document) = 0;
    };

    class LoginData : public Data {
    public:
	std::string accessToken;
	std::string tokenType;

	bool load(rapidjson::Document& document) override;
    };

    class UploadedFileData : public Data {
    public:
	std::string id;
	std::string filePath;
	bool accepted;
	std::vector<std::string> errors;
	std::string zipFilePath;

	bool load(rapidjson::Document& document) override;
    };

    struct Steps
    {
	std::string datetime_utc;
	std::string datetime_local;
	int steps;
	float distance;
	int calories;
    };

    class StepsData : public Data {
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

    class SleepData : public Data {
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
	httplib::Error httplibError = httplib::Error::Success;

    public:
	Error() {}
	Error(const ErrorType error, const httplib::Error httplibError = httplib::Error::Success)
	    : error(error), httplibError(httplibError) {}

	bool hasError() const;
	std::string errorToString() const;
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
	bool isValid() const { return !this->error.hasError(); }
	const Error getError() const { return error; }
	const T& getData() { return *this->data; }
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

	const fitgalgo::Result<fitgalgo::UploadedFileData> doPostForFile(
	    httplib::Client& client, const std::filesystem::path& file_path) const;

    public:
	Connection(const std::string host, const int port)
	    : host(host), port(port) {}
	const fitgalgo::Result<fitgalgo::LoginData> login(
	    const std::string& username, const std::string& password);
	void logout();
	bool hasToken() const;
	const std::vector<fitgalgo::Result<fitgalgo::UploadedFileData>> postFile(
	    std::filesystem::path& path) const;
	const fitgalgo::Result<fitgalgo::StepsData> getSteps() const;
	const fitgalgo::Result<fitgalgo::SleepData> getSleep() const;
    };
}

#endif // _ES_RGMF_FITGALGO_H
