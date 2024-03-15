#ifndef _ES_RGMF_CORE_API_H
#define _ES_RGMF_CORE_API_H 1

#include <cstddef>
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

enum class ActivityType {
  GENERIC,
  DISTANCE,
  SPLITS,
  SETS
};

std::ostream& operator<<(std::ostream& os, ActivityType at);

struct Activity
{
    std::string zone_info{};
    std::string username{};
    std::string sport_profile_name{};
    std::string sport{};
    std::string sub_sport{};
    std::optional<std::pair<float, float>> start_lat_lon{};
    std::optional<std::pair<float, float>> end_lat_lon{};
    std::string start_time_utc{};
    std::optional<float> total_elapsed_time{}; 
    std::optional<float> total_timer_time{};
    std::optional<float> total_work_time{};
    std::optional<float> total_distance{};
    std::optional<float> avg_speed{};
    std::optional<float> max_speed{};
    std::optional<float> avg_cadence{};
    std::optional<float> max_cadence{};
    std::optional<float> avg_running_cadence{};
    std::optional<float> max_running_cadence{};
    std::optional<float> total_strides{};
    std::optional<float> total_calories{};
    std::optional<float> total_ascent{};
    std::optional<float> total_descent{};
    std::optional<float> avg_temperature{};
    std::optional<float> max_temperature{};
    std::optional<float> min_temperature{};
    std::optional<float> avg_respiration_rate{};
    std::optional<float> max_respiration_rate{};
    std::optional<float> min_respiration_rate{};
    std::optional<float> training_load_peak{};
    std::optional<float> total_training_effect{};
    std::optional<float> total_anaerobic_training_effect{};

    virtual ActivityType get_id() const;
};

enum class SetType {
  UNKNOWN = 0,
  ACTIVE,
  REST
};
const std::string set_type_names[] = { "unknown", "active", "rest" };

struct Set
{
    float timestamp{};
    SetType set_type{SetType::UNKNOWN};
    float duration{};
    int repetitions{};
    float weight{};
    float start_time{};
    std::vector<std::string> category{};
    std::vector<std::string> category_subtype{};
    std::string weight_display_unit{};
    int message_index{};
    int wkt_step_index{};
};

struct SetsActivity : public Activity
{
    std::vector<Set> sets{};

    ActivityType get_id() const override;
};

enum class SplitResult {
  DISCARDED = 0,
  UNKNOWN,
  ATTEMPTED,
  COMPLETED
};
const std::string split_result_names[] = { "discarded", "unknown", "attempted", "completed" };

struct Split
{
    std::string split_type{};
    float total_elapsed_time{};
    float total_timer_time{};
    float start_time{};
    int avg_hr{};
    int max_hr{};
    int total_calories{};
    int difficulty{};
    SplitResult result{SplitResult::UNKNOWN};
};

struct SplitsActivity : public Activity
{
    std::vector<Split> splits{};

    ActivityType get_id() const override;
};

struct Record
{
    float timestamp{};
    std::string activity_type{};
    std::pair<float, float> position_lat_lon{};
    int gps_accuracy{};
    float distance{};
    float speed{};
    int vertical_speed{};
    int power{};
    float altitude{};
    int ascent_rate{};
    int grade{};
    int heart_rate{};
    int cadence{};
    int temperature{};
    int calories{};
    int total_cycles{};
    int step_length{};
    int absolute_pressure{};
    float respiration_rate{};
    int current_stress{};
    int zone{};
};

struct Lap
{
    int message_index{};

    float timestamp{};
    float start_time{};

    std::pair<float, float> start_lat_lon{};
    std::pair<float, float> end_lat_lon{};

    float total_elapsed_time{};
    float total_timer_time{};
    float total_moving_time{};

    float total_distance{};

    float avg_speed{};
    float max_speed{};

    int avg_heart_rate{};
    int max_heart_rate{};
    int min_heart_rate{};

    int avg_cadence{};
    int max_cadence{};
    int avg_running_cadence{};
    int max_running_cadence{};

    int total_ascent{};
    int total_descent{};
    int avg_altitude{};
    int max_altitude{};
    int min_altitude{};
    int avg_grade{};
    int avg_pos_grade{};
    int avg_neg_grade{};
    int max_pos_grade{};
    int max_neg_grade{};

    int total_strides{};

    int total_calories{};
    int total_fat_calories{};

    int intensity{};

    int avg_temperature{};
    int max_temperature{};
    int min_temperature{};

    int avg_respiration_rate{};
    int max_respiration_rate{};
};

struct DistanceActivity : public Activity
{
    std::vector<Record> records{};
    std::vector<Lap> laps{};

    ActivityType get_id() const override;
};

struct ActivitiesData : public Data
{
    std::map<DateIdx, std::unique_ptr<Activity>> activities{};
    std::vector<std::string> errors{};

    ActivitiesData() : activities(), errors() {}
    ActivitiesData(const ActivitiesData& other);

    ActivitiesData& operator=(const ActivitiesData& other);

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
