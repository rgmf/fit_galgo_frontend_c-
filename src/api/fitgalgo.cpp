#include <rapidjson/document.h>
#include <httplib/httplib.h>

#include "fitgalgo.h"

bool fitgalgo::Error::hasError() const
{
    return this->error != ErrorType::Success || this->httplibError != httplib::Error::Success;
}

std::string fitgalgo::Error::errorToString() const
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
    if (errorString != errorMessages.end()) {
	result = errorString->second;
    }

    if (httplibError != httplib::Error::Success) {
	result += '\n';
	result += httplib::to_string(httplibError);
    }

    if (result.empty()) {
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

bool fitgalgo::LoginData::load(rapidjson::Document& document)
{
    const auto accessTokenMember = document.FindMember("access_token");
    if (accessTokenMember == document.MemberEnd() || !accessTokenMember->value.IsString()) {
	return false;
    }
    this->accessToken = accessTokenMember->value.GetString();

    const auto v = document.FindMember("token_type");
    if (v != document.MemberEnd() && v->value.IsString()) {
	this->tokenType = v->value.GetString();
    }

    return true;
}

bool fitgalgo::UploadedFileData::load(rapidjson::Document& document)
{
    const auto data = document.FindMember("data");
    if (data == document.MemberEnd() || !data->value.IsArray()) {
	return false;
    }

    const auto itr = data->value.GetArray().Begin();
    const auto id = itr->FindMember("id");
    const auto filePath = itr->FindMember("file_path");
    const auto accepted = itr->FindMember("accepted");
    const auto zip_file_path = itr->FindMember("zip_file_path");
    const auto errors = itr->FindMember("errors");

    if (id != itr->MemberEnd() && id->value.IsString()) {
	this->id = id->value.GetString();
    }
    if (filePath != itr->MemberEnd() && filePath->value.IsString()) {
	this->filePath = filePath->value.GetString();
    }
    if (accepted != itr->MemberEnd() && accepted->value.IsBool()) {
	this->accepted = accepted->value.GetBool();
    }
    if (zip_file_path != itr->MemberEnd() && zip_file_path->value.IsString()) {
	this->zipFilePath = zip_file_path->value.GetString();
    }
    if (errors != itr->MemberEnd() && errors->value.IsArray()) {
	for (const auto& v : errors->value.GetArray()) {
	    if (v.IsString()) {
		this->errors.emplace_back(v.GetString());
	    }
	}
    }

    return true;
}

bool fitgalgo::StepsData::load(rapidjson::Document& document)
{
    const auto data = document.FindMember("data");
    if (data == document.MemberEnd() || !data->value.IsArray()) {
	return false;
    }

    for (const auto& v : data->value.GetArray()) {
	if (v.IsObject() && v.HasMember("datetime_local")) {
	    auto itr_dt_utc = v.FindMember("datetime_utc");
	    auto itr_dt_local = v.FindMember("datetime_local");
	    auto itr_steps = v.FindMember("total_steps");
	    auto itr_distance = v.FindMember("total_distance");
	    auto itr_calories = v.FindMember("total_calories");

	    struct fitgalgo::Steps item{};

	    item.datetime_local = itr_dt_local->value.GetString();
	    if (itr_dt_utc != v.MemberEnd() && itr_dt_utc->value.IsString()) {
		item.datetime_utc = itr_dt_utc->value.GetString();
	    }
	    if (itr_steps != v.MemberEnd() && itr_steps->value.IsInt()) {
		item.steps = itr_steps->value.GetInt();
	    }
	    if (itr_distance != v.MemberEnd() && itr_distance->value.IsFloat()) {
		item.distance = itr_distance->value.GetFloat();
	    }
	    if (itr_calories != v.MemberEnd() && itr_calories->value.IsInt()) {
		item.calories = itr_calories->value.GetInt();
	    }
		
	    this->steps[item.datetime_local.substr(0, 10)] = item;
	} else {
	    this->errors.emplace_back(
		"JSON error: a datetime_local item is not an object or not exists.");
	}
    }

    return true;
}

template<typename T>
void fitgalgo::Result<T>::load(const httplib::Result &response)
{   
    if (!response) {
	this->error = fitgalgo::Error(fitgalgo::ErrorType::NotResponse);
	return;
    }

    this->status = response->status;

    if (response->status < 200) {
	this->error = fitgalgo::Error(fitgalgo::ErrorType::Http100, response.error());
	return;
    }

    if (response->status >= 300 && response->status < 400) {
	this->error = fitgalgo::Error(fitgalgo::ErrorType::Http300, response.error());
	return;
    }

    if (response->status == 401) {
	this->error = fitgalgo::Error(fitgalgo::ErrorType::Http401, response.error());
	return;
    }

    if (response->status > 401 && response->status < 500) {
	this->error = fitgalgo::Error(fitgalgo::ErrorType::Http400, response.error());
	return;
    }

    if (response->status >= 500) {
	this->error = fitgalgo::Error(fitgalgo::ErrorType::Http500, response.error());
	return;
    }

    rapidjson::Document document;
    document.Parse(response->body.c_str());
    this->data = std::make_unique<T>();
    bool isValid = this->data->load(document);

    if (isValid) {
	this->error = fitgalgo::Error(fitgalgo::ErrorType::Success, response.error());
    } else {
	this->error = fitgalgo::Error(fitgalgo::ErrorType::NotData, response.error());
    }
}

template<typename T>
void fitgalgo::Result<T>::load(const T& newData)
{
    this->data = std::make_unique<T>(newData);
}

const fitgalgo::Result<fitgalgo::LoginData> fitgalgo::Connection::login(
    const std::string& username, const std::string& password)
{
    httplib::Client client(this->host, this->port);
    std::stringstream credentials;
    rapidjson::Document document;

    credentials << "username=" << username << "&password=" << password;
    auto response = client.Post(
	"/auth/login", credentials.str(), "application/x-www-form-urlencoded");

    fitgalgo::Result<fitgalgo::LoginData> result;
    result.load(response);

    if (result.isValid()) {
	this->token = result.getData().accessToken;
    }

    return result;
}

void fitgalgo::Connection::logout() { this->token = ""; }

bool fitgalgo::Connection::hasToken() const { return !this->token.empty(); }

const fitgalgo::Result<fitgalgo::UploadedFileData> fitgalgo::Connection::doPostForFile(
    httplib::Client& client, const std::filesystem::path& path) const
{
    try {
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
	fitgalgo::Result<fitgalgo::UploadedFileData> result;
	result.load(response);
	return result;
    } catch (const std::exception& e) {
	fitgalgo::UploadedFileData ufd;
	ufd.filePath = path.string();
	ufd.accepted = false;
	ufd.errors = {e.what()};

        fitgalgo::Result<fitgalgo::UploadedFileData> result;
	result.load(ufd);

	return result;
    }
}

const std::vector<fitgalgo::Result<fitgalgo::UploadedFileData>> fitgalgo::Connection::postFile(
    std::filesystem::path& path) const
{
    std::vector<fitgalgo::Result<fitgalgo::UploadedFileData>> results;

    httplib::Client client(this->host, this->port);
    client.set_bearer_token_auth(this->token);
    client.set_connection_timeout(0, 1000000); // 1000 milliseconds = 1 second
    client.set_read_timeout(10, 0); // 10 seconds
    client.set_write_timeout(10, 0); // 10 seconds

    if (std::filesystem::is_regular_file(path)) {
	auto result = this->doPostForFile(client, path);
	results.emplace_back(std::move(result));
    } else if (std::filesystem::is_directory(path)) {
	for (const auto& e : std::filesystem::directory_iterator(path)) {
	    auto p = e.path();
	    auto result = this->doPostForFile(client, p);
	    results.emplace_back(std::move(result));
	}
    }

    return results;
}

const fitgalgo::Result<fitgalgo::StepsData> fitgalgo::Connection::getSteps() const
{
    httplib::Client client(this->host, this->port);
    client.set_bearer_token_auth(this->token);
    auto response = client.Get("/monitorings/steps");
    
    fitgalgo::Result<fitgalgo::StepsData> result;
    result.load(response);
    return result;
}
