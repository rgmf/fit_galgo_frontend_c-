#ifndef _ES_RGMF_CORE_API_H
#define _ES_RGMF_CORE_API_H 1

#include <memory>
#include <sys/types.h>
#include <chrono>
#include <filesystem>
#include <string>
#include <map>
#include <vector>

#include <httplib/httplib.h>
#include <rapidjson/document.h>

namespace fitgalgo
{

struct Data
{
    virtual ~Data() = default;
    virtual bool load(const rapidjson::Document& document) = 0;
};

/**
 * This class handle a datetime in ISO-8601 format (without zone info) in an
 * string format.
 *
 * Its constructor expects these possibles formats:
 * - yyyy-mm-dd
 * - yyyy-mm-ddThh:mm:ss
 * - yyyy-mm-dd hh:mm:ss
 *
 * These formats could contains more trailing extra characteres but they will
 * be ignored.
 *
 * In all cases, it keeps the datetime with this format: yyyy-mm-ddThh:mm:ss
 *
 * If hour, minutes and seconds are not provided then it will use 00:00:00
 *
 * If you try to create an object with an invalid string, then it will generate
 * an exception.
 */
class DateIdx
{
private:
    std::string datetime{};

    inline void set_datetime_if_valid_value(const std::string& value, const std::string& format);
    inline void decrement_date();

public:
    explicit DateIdx() : datetime() {}
    explicit DateIdx(const std::string& value);
    const std::string& value() const;
    std::chrono::year_month_day ymd() const;
    short year() const;
    short month() const;
    short day() const;
    bool is_valid() const;
    DateIdx& operator--(int);
    DateIdx& operator--();
    friend bool operator<(const DateIdx& l, const DateIdx& r);
};

struct LoginData : public Data
{
    std::string access_token{};
    std::string token_type{};

    bool load(const rapidjson::Document& document) override;
};

struct UploadedFile
{
    std::string id{};
    std::string filename{};
    bool accepted{};
    std::vector<std::string> errors{};
    std::string zip_filename{};
};
    
struct UploadedFileData : public Data
{
    std::vector<UploadedFile> uploaded_files{};
    std::vector<std::string> errors{};

    bool load(const rapidjson::Document& document) override;
};

struct Steps
{
    std::string datetime_utc{};
    std::string datetime_local{};
    int steps{};
    float distance{};
    int calories{};

    Steps& operator+=(const Steps& rhs);
    friend Steps operator+(Steps lhs, const Steps& rhs);
};

struct StepsData : public Data
{
    std::map<DateIdx, Steps> steps{};
    std::vector<std::string> errors{};

    bool load(const rapidjson::Document& document) override;
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

    SleepAssessment& operator+=(const SleepAssessment& rhs);
    friend SleepAssessment operator+(SleepAssessment lhs, const SleepAssessment& rhs);
    SleepAssessment& operator/(int n);
};

struct SleepLevel
{
    std::string datetime_utc{};
    std::string level{};
};

struct Sleep
{
    std::string zone_info{};
    SleepAssessment assessment{};
    std::vector<SleepLevel> levels{};
    std::vector<std::string> dates{};

    bool is_early_morning() const;
    Sleep& operator+=(const Sleep& rhs);
    friend Sleep operator+(Sleep lhs, const Sleep& rhs);
    Sleep& operator/(int n);
};

struct SleepWithCount
{
    Sleep sleep{};
    size_t count{};
};

struct SleepData : public Data
{
    std::map<DateIdx, Sleep> sleep{};
    std::vector<std::string> errors{};

    bool load(const rapidjson::Document& document) override;
};

struct Activity
{
    std::string zone_info{};
    std::string username{};
    std::string sport_profile_name{};
    std::string sport{};
    std::string sub_sport{};
    std::pair<float, float> start_lat_lon{};
    std::pair<float, float> end_lat_lon{};
    std::string start_time_utc{};
    float total_elapsed_time{};
    float total_timer_time{};
    float total_distance{};
    float avg_speed{};
    float max_speed{};
    float avg_cadence{};
    float max_cadence{};
    float avg_running_cadence{};
    float max_running_cadence{};
    float total_strides{};
    float total_calories{};
    float total_ascent{};
    float total_descent{};
    float avg_temperature{};
    float max_temperature{};
    float min_temperature{};
    float avg_respiration_rate{};
    float max_respiration_rate{};
    float min_respiration_rate{};
    float training_load_peak{};
    float total_training_effect{};
    float total_anaerobic_training_effect{};

    Activity& operator+=(const Activity& rhs);
    friend Activity operator+(Activity lhs, const Activity& rhs);
};

struct ActivitiesData : public Data
{
    std::map<DateIdx, Activity> activities{};
    std::vector<std::string> errors{};

    bool load(const rapidjson::Document& document) override;
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
    ErrorType error;
    httplib::Error httplib_error;

public:
    explicit Error() : error(ErrorType::NotLoaded), httplib_error(httplib::Error::Success) {}
    explicit Error(
	const ErrorType error, const httplib::Error httplib_error = httplib::Error::Success)
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
    explicit Result();
    Result(const Result<T>& other);
    Result(Result<T>&& other) noexcept;

    Result<T>& operator=(const Result<T>& other);
    Result<T>& operator=(const Result<T>&& other) noexcept;

    void load(const httplib::Result& response);
    void load(const T& newData);
    bool is_valid() const { return !this->error.has_error(); }
    const Error get_error() const { return error; }
    const T& get_data() const { return *this->data; }
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

    const Result<UploadedFileData> do_post_for_file(
	httplib::Client& client, const std::filesystem::path& file_path) const;

public:
    explicit Connection(const std::string host, const int port)
	: host(host), port(port) {}
    const Result<LoginData> login(
	const std::string& username, const std::string& password);
    void logout();
    bool has_token() const;
    const std::vector<Result<UploadedFileData>> post_file(
	std::filesystem::path& path) const;
    const Result<StepsData> get_steps() const;
    const Result<SleepData> get_sleep() const;
    const Result<ActivitiesData> get_activities() const;
};

} // namespace fitgalgo

#endif // _ES_RGMF_CORE_API_H
