#ifndef _ES_RGMF_FITGALGO_H
#define _ES_RGMF_FITGALGO_H 1

#include <memory>
#include <string>
#include <map>
#include <vector>
#include <filesystem>
#include <optional>

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

	std::optional<std::string> getStringMember(std::string) const;
	std::optional<int> getIntMember(std::string) const;
	std::optional<rapidjson::GenericArray<true, rapidjson::Value::ValueType>>
	getArrayMember(std::string memberName) const;

    public:
        // Result() : error(), status(), data(std::make_unique<T>()) {}
	// Result(Error e, int s, std::unique_ptr<T> d)
	//     : error(e), status(s), data(std::make_unique<T>(d)) {}
	void load(const httplib::Result& response);
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
	 * Handles POST file response.
	 *
	 * @param response Response from httplib::Result library.
	 * @param file_path String with the path of the file that tried to
	 * upload.
	 *
	 * @return fitgalgo::Result.
	 */
	fitgalgo::Result<fitgalgo::UploadedFileData> handlePostFileResponse(
	    const httplib::Result& response);

	/**
	 * Does POST for file path passed as parameter.
	 *
	 * @param httplib::Client Client object reference to do the POST.
	 * @param std::filesystem::path Path object, the file that will be
	 * posted (uploaded).
	 *
	 * @return fitgalgo::Result.
	 */
	fitgalgo::Result<fitgalgo::UploadedFileData> doPostForFile(
	    httplib::Client& client, const std::filesystem::path& file_path);

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
	//std::vector<UploadedFile> postFile(std::filesystem::path& path) const;
	std::vector<fitgalgo::Result<fitgalgo::UploadedFileData>> postFile(
	    std::filesystem::path& path);

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
