#include <rapidjson/document.h>
#include <httplib/httplib.h>

#include "api.h"

namespace fitgalgo
{    

bool Error::has_error() const
{
    return this->error != ErrorType::Success || this->httplib_error != httplib::Error::Success;
}

std::string Error::error_to_string() const
{
    static const std::map<ErrorType, std::string> errorMessages = {
        {ErrorType::Success, "Success response."},
        {ErrorType::NotLoaded, "Error: response not loaded."},
        {ErrorType::Unknown, "Error: unknown."},
        {ErrorType::NotResponse, "Error: response not received."},
        {ErrorType::NotData, "Error: there is no data in the response received."},
        {ErrorType::Http100, "Error: 1xx code error."},
        {ErrorType::Http300, "Error: 3xx code error."},
        {ErrorType::Http400, "Error: 4xx code error."},
	{ErrorType::Http401, "Error: authentication error."},
        {ErrorType::Http500, "Error: 5xx code error."}
    };

    std::string result;

    auto errorString = errorMessages.find(error);
    if (errorString != errorMessages.end())
    {
	result = errorString->second;
    }

    if (httplib_error != httplib::Error::Success)
    {
	result += '\n';
	result += httplib::to_string(httplib_error);
    }

    if (result.empty())
    {
	return "Error: Unknown error.";
    }

    return result;
    // const auto v = this->document.FindMember("detail");
    // if (v != this->document.MemberEnd() && v->value.IsString()) {
    // 	return "Error: " + std::string(v->value.GetString()) + ".";
    // } else {
    // 	return "Error: API unknown error.";
    // }
}

bool LoginData::load(rapidjson::Document& document)
{
    const auto at_member = document.FindMember("access_token");
    if (at_member == document.MemberEnd() || !at_member->value.IsString())
    {
	return false;
    }
    this->access_token = at_member->value.GetString();

    const auto v = document.FindMember("token_type");
    if (v != document.MemberEnd() && v->value.IsString())
    {
	this->token_type = v->value.GetString();
    }

    return true;
}

bool UploadedFileData::load(rapidjson::Document& document)
{
    const auto data = document.FindMember("data");
    if (data == document.MemberEnd() || !data->value.IsArray())
    {
	return false;
    }

    const auto itr = data->value.GetArray().Begin();
    const auto id = itr->FindMember("id");
    const auto file_path = itr->FindMember("file_path");
    const auto accepted = itr->FindMember("accepted");
    const auto zip_file_path = itr->FindMember("zip_file_path");
    const auto errors = itr->FindMember("errors");

    if (id != itr->MemberEnd() && id->value.IsString())
    {
	this->id = id->value.GetString();
    }

    if (file_path != itr->MemberEnd() && file_path->value.IsString())
    {
	this->file_path = file_path->value.GetString();
    }

    if (accepted != itr->MemberEnd() && accepted->value.IsBool())
    {
	this->accepted = accepted->value.GetBool();
    }

    if (zip_file_path != itr->MemberEnd() && zip_file_path->value.IsString())
    {
	this->zip_file_path = zip_file_path->value.GetString();
    }

    if (errors != itr->MemberEnd() && errors->value.IsArray())
    {
	for (const auto& v : errors->value.GetArray())
	{
	    if (v.IsString())
	    {
		this->errors.emplace_back(v.GetString());
	    }
	}
    }

    return true;
}

bool StepsData::load(rapidjson::Document& document)
{
    const auto data = document.FindMember("data");
    if (data == document.MemberEnd() || !data->value.IsArray())
    {
	return false;
    }

    for (const auto& v : data->value.GetArray())
    {
	if (v.IsObject() &&
	    v.HasMember("datetime_local") &&
	    DateIdx::is_valid(v["datetime_local"].GetString()))
	{
	    auto itr_dt_utc = v.FindMember("datetime_utc");
	    auto itr_dt_local = v.FindMember("datetime_local");
	    auto itr_steps = v.FindMember("total_steps");
	    auto itr_distance = v.FindMember("total_distance");
	    auto itr_calories = v.FindMember("total_calories");

	    struct Steps item{};

	    item.datetime_local = itr_dt_local->value.GetString();

	    if (itr_dt_utc != v.MemberEnd() && itr_dt_utc->value.IsString())
		item.datetime_utc = itr_dt_utc->value.GetString();

	    if (itr_steps != v.MemberEnd() && itr_steps->value.IsInt())
		item.steps = itr_steps->value.GetInt();

            if (itr_distance != v.MemberEnd() && itr_distance->value.IsFloat())
		item.distance = itr_distance->value.GetFloat();

	    if (itr_calories != v.MemberEnd() && itr_calories->value.IsInt())
		item.calories = itr_calories->value.GetInt();

	    DateIdx idx(item.datetime_local);
	    this->steps[idx] = item;
	}
	else
	{
	    this->errors.emplace_back(
		"JSON error: a datetime_local item is not an object, not exists or is not valid");
	}
    }

    return true;
}

bool SleepData::load(rapidjson::Document& document)
{
    const auto data = document.FindMember("data");
    if (data == document.MemberEnd() || !data->value.IsArray())
    {
	return false;
    }

    for (const auto& v : data->value.GetArray())
    {
	if (v.IsObject())
	{
	    auto itrZoneInfo = v.FindMember("zone_info");
	    auto itrAssessment = v.FindMember("assessment");
	    auto itrLevels = v.FindMember("levels");
	    auto itrDates = v.FindMember("dates");

	    struct Sleep sleepItem{};

	    if (itrZoneInfo != v.MemberEnd() && itrZoneInfo->value.IsString())
	    {
		sleepItem.zone_info = itrZoneInfo->value.GetString();
	    }

	    if (itrAssessment != v.MemberEnd() && itrAssessment->value.IsObject())
	    {
		auto itr_cas = itrAssessment->value.FindMember("combined_awake_score");
		auto itr_ats = itrAssessment->value.FindMember("awake_time_score");
		auto itr_acs = itrAssessment->value.FindMember("awakenings_count_score");
		auto itr_dss = itrAssessment->value.FindMember("deep_sleep_score");
		auto itr_sds = itrAssessment->value.FindMember("sleep_duration_score");
		auto itr_lss = itrAssessment->value.FindMember("light_sleep_score");
		auto itr_oss = itrAssessment->value.FindMember("overall_sleep_score");
		auto itr_sqs = itrAssessment->value.FindMember("sleep_quality_score");
		auto itr_srs = itrAssessment->value.FindMember("sleep_recovery_score");
		auto itr_rss = itrAssessment->value.FindMember("rem_sleep_score");
		auto itr_sres = itrAssessment->value.FindMember("sleep_restlessness_score");
		auto itr_ac = itrAssessment->value.FindMember("awakenings_count");
		auto itr_is = itrAssessment->value.FindMember("interruptions_score");
		auto itr_asds = itrAssessment->value.FindMember("average_stress_during_sleep");

		struct SleepAssessment assessment{};

		if (itr_cas != itrAssessment->value.MemberEnd() && itr_cas->value.IsInt())
		    assessment.combined_awake_score = itr_cas->value.GetInt();

		if (itr_ats != itrAssessment->value.MemberEnd() && itr_ats->value.IsInt())
		    assessment.awake_time_score = itr_ats->value.GetInt();

		if (itr_acs != itrAssessment->value.MemberEnd() && itr_acs->value.IsInt())
		    assessment.awakenings_count_score = itr_acs->value.GetInt();

		if (itr_dss != itrAssessment->value.MemberEnd() && itr_dss->value.IsInt())
		    assessment.deep_sleep_score = itr_dss->value.GetInt();

		if (itr_sds != itrAssessment->value.MemberEnd() && itr_sds->value.IsInt())
		    assessment.sleep_duration_score = itr_sds->value.GetInt();

		if (itr_lss != itrAssessment->value.MemberEnd() && itr_lss->value.IsInt())
		    assessment.light_sleep_score = itr_lss->value.GetInt();

		if (itr_oss != itrAssessment->value.MemberEnd() && itr_oss->value.IsInt())
		    assessment.overall_sleep_score = itr_oss->value.GetInt();

		if (itr_sqs != itrAssessment->value.MemberEnd() && itr_sqs->value.IsInt())
		    assessment.sleep_quality_score = itr_sqs->value.GetInt();
		
		if (itr_srs != itrAssessment->value.MemberEnd() && itr_srs->value.IsInt())
		    assessment.sleep_recovery_score = itr_srs->value.GetInt();

		if (itr_rss != itrAssessment->value.MemberEnd() && itr_rss->value.IsInt())
		    assessment.rem_sleep_score = itr_rss->value.GetInt();

		if (itr_sres != itrAssessment->value.MemberEnd() && itr_sres->value.IsInt())
		    assessment.sleep_restlessness_score = itr_sres->value.GetInt();

		if (itr_ac != itrAssessment->value.MemberEnd() && itr_ac->value.IsInt())
		    assessment.awakenings_count = itr_ac->value.GetInt();

		if (itr_is != itrAssessment->value.MemberEnd() && itr_is->value.IsInt())
		    assessment.interruptions_score = itr_is->value.GetInt();

		if (itr_asds != itrAssessment->value.MemberEnd() && itr_asds->value.IsFloat())
		    assessment.average_stress_during_sleep = itr_asds->value.GetFloat();

		sleepItem.assessment = assessment;
	    }

	    if (itrLevels != v.MemberEnd() && itrLevels->value.IsArray())
	    {
		std::vector<SleepLevel> levels{};
		for (const auto& l : itrLevels->value.GetArray())
		{
		    if (l.IsObject())
		    {
			auto itr_dt_utc = l.FindMember("datetime_utc");
			auto itr_level = l.FindMember("level");

			SleepLevel level{};

			if (itr_dt_utc != l.MemberEnd() && itr_dt_utc->value.IsString())
			    level.datetime_utc = itr_dt_utc->value.GetString();

			if (itr_level != l.MemberEnd() && itr_level->value.IsString())
			    level.level = itr_level->value.GetString();

			levels.emplace_back(level);
		    }
		}
		
		sleepItem.levels = levels;
	    }

	    std::string idxDates{};
	    if (itrDates != v.MemberEnd() && itrDates->value.IsArray())
	    {
		std::vector<std::string> dates{};
		for (const auto& d : itrDates->value.GetArray())
		{
		    if (d.IsString())
		    {
			sleepItem.dates.emplace_back(d.GetString());
			idxDates += d.GetString();
		    }
		}
	    }

	    if (!idxDates.empty() && DateIdx::is_valid(idxDates))
		this->sleep[DateIdx(idxDates)] = sleepItem;
	    else
		this->errors.emplace_back("JSON error: dates are no valid or are not dates.");
	}
	else
	{
	    this->errors.emplace_back(
		"JSON error: it expects an object with sleep information.");
	}
    }

    return true;
}

template <typename T>
void Result<T>::load(const httplib::Result &response)
{   
    if (!response)
    {
	this->error = Error(ErrorType::NotResponse);
	return;
    }

    this->status = response->status;

    if (response->status < 200)
    {
	this->error = Error(ErrorType::Http100, response.error());
	return;
    }

    if (response->status >= 300 && response->status < 400)
    {
	this->error = Error(ErrorType::Http300, response.error());
	return;
    }

    if (response->status == 401)
    {
	this->error = Error(ErrorType::Http401, response.error());
	return;
    }

    if (response->status > 401 && response->status < 500)
    {
	this->error = Error(ErrorType::Http400, response.error());
	return;
    }

    if (response->status >= 500)
    {
	this->error = Error(ErrorType::Http500, response.error());
	return;
    }

    rapidjson::Document document;
    document.Parse(response->body.c_str());
    this->data = std::make_unique<T>();
    bool is_valid = this->data->load(document);

    if (is_valid)
	this->error = Error(ErrorType::Success, response.error());
    else
	this->error = Error(ErrorType::NotData, response.error());
}

template <typename T>
void Result<T>::load(const T& newData)
{
    this->data = std::make_unique<T>(newData);
}

const Result<LoginData> Connection::login(
    const std::string& username, const std::string& password)
{
    httplib::Client client(this->host, this->port);
    std::stringstream credentials;
    rapidjson::Document document;

    credentials << "username=" << username << "&password=" << password;
    auto response = client.Post(
	"/auth/login", credentials.str(), "application/x-www-form-urlencoded");

    Result<LoginData> result;
    result.load(response);

    if (result.is_valid())
	this->token = result.get_data().access_token;

    return result;
}

void Connection::logout() { this->token = ""; }

bool Connection::has_token() const { return !this->token.empty(); }

const Result<UploadedFileData> Connection::do_post_for_file(
    httplib::Client& client, const std::filesystem::path& path) const
{
    try
    {
	// Open and read the file in a buffer.
	std::ifstream file(path.string(), std::ios::binary);
	std::stringstream buffer;
	buffer << file.rdbuf();
	file.close();

	// Send POST request with the file.
	httplib::MultipartFormDataItems items = {
	    { "files", buffer.str(), path.filename().string(), "application/octet-stream" },
	    { "zone", "Europe/Madrid", "", "" }
	};

	auto response = client.Post("/files/", items);
	Result<UploadedFileData> result;
	result.load(response);
	return result;
    }
    catch (const std::exception& e)
    {
	UploadedFileData ufd;
	ufd.file_path = path.string();
	ufd.accepted = false;
	ufd.errors = {e.what()};

        Result<UploadedFileData> result;
	result.load(ufd);

	return result;
    }
}

const std::vector<Result<UploadedFileData>> Connection::post_file(
    std::filesystem::path& path) const
{
    std::vector<Result<UploadedFileData>> results;

    httplib::Client client(this->host, this->port);
    client.set_bearer_token_auth(this->token);
    client.set_connection_timeout(0, 1000000); // 1000 milliseconds = 1 second
    client.set_read_timeout(10, 0); // 10 seconds
    client.set_write_timeout(10, 0); // 10 seconds

    if (std::filesystem::is_regular_file(path))
    {
	auto result = this->do_post_for_file(client, path);
	results.emplace_back(std::move(result));
    }
    else if (std::filesystem::is_directory(path))
    {
	for (const auto& e : std::filesystem::directory_iterator(path))
	{
	    auto p = e.path();
	    auto result = this->do_post_for_file(client, p);
	    results.emplace_back(std::move(result));
	}
    }

    return results;
}

const Result<StepsData> Connection::get_steps() const
{
    httplib::Client client(this->host, this->port);
    client.set_bearer_token_auth(this->token);
    auto response = client.Get("/monitorings/steps");
    
    Result<StepsData> result;
    result.load(response);
    return result;
}

const Result<SleepData> Connection::get_sleep() const
{
    httplib::Client client(this->host, this->port);
    client.set_bearer_token_auth(this->token);
    auto response = client.Get("/monitorings/sleep");
    
    Result<SleepData> result;
    result.load(response);
    return result;
}

} // namespace fitgalgo
