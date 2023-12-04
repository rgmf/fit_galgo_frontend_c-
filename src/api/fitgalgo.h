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

    template<typename T>
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

	/**
	 * Does POST for file path passed as parameter.
	 *
	 * @param httplib::Client Client object reference to do the POST.
	 * @param std::filesystem::path Path object, the file that will be
	 * posted (uploaded).
	 *
	 * @return fitgalgo::Result.
	 */
	const fitgalgo::Result<fitgalgo::UploadedFileData> doPostForFile(
	    httplib::Client& client, const std::filesystem::path& file_path) const;

    public:
	/**
	 * Default constructor.
	 */
	Connection(const std::string host, const int port)
	    : host(host), port(port) {}
	
	/**
	 * Log FitGalgo API in.
	 *
	 * @param username Username for login.
	 * @param password User's password for login.
	 *
	 * @return bool True if login was okay; false otherwise.
	 */
	const fitgalgo::Result<fitgalgo::LoginData> login(
	    const std::string& username, const std::string& password);

	/**
	 * Clean login information.
	 */
	void logout();

	/**
	 * @return bool True if there is token; false otherwise.
	 */
	bool hasToken() const;

	/**
	 * Does POST for uploading FIT files.
	 */
	const std::vector<fitgalgo::Result<fitgalgo::UploadedFileData>> postFile(
	    std::filesystem::path& path) const;

	/**
	 * Does GET for getting steps information.
	 *
	 * @throws std::runtime_error If an HTTP or JSON error occurs during the
	 * request.
	 *
	 * @return std::map<std::string, Steps> A map with local datetime as key
	 * and Steps struct as value.
	 */
	const fitgalgo::Result<fitgalgo::StepsData> getSteps() const;
    };
}

#endif // _ES_RGMF_FITGALGO_H
