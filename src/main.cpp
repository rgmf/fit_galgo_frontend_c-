#include <algorithm>
#include <cstring>
#include <iostream>
#include <iterator>
#include <limits>
#include <map>
#include <string>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <chrono>

#include <unistd.h>
#include <termios.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include <rapidjson/document.h>
#include <httplib/httplib.h>

#include "net/HttpClient.h"
#include "net/HttpClientException.h"
#include "config/Context.h"

#define BUFFER_SIZE 1024

/**
 * @return Message with information and help about programm usage.
 */
std::string usage_help()
{
    return "Usage: fitgalgo -h [host] -p [port]";
}

/**
 * Disables console characters impress.
 */
void disableEcho() {
    struct termios tty;
    tcgetattr(STDIN_FILENO, &tty);
    tty.c_lflag &= ~ECHO;
    tcsetattr(STDIN_FILENO, TCSANOW, &tty);
}

/**
 * Enables console characters impress.
 */
void enableEcho() {
    struct termios tty;
    tcgetattr(STDIN_FILENO, &tty);
    tty.c_lflag |= ECHO;
    tcsetattr(STDIN_FILENO, TCSANOW, &tty);
}

/**
 * From datetime in UTC format to date in the specified zone.
 */
std::string from_utc_to_zone(const std::string dtUtc, const std::string ianaZone) {
    std::string date_time_format = "%Y-%m-%dT%H:%M:%S.000Z";
    std::istringstream ss{ dtUtc };
    std::tm dt;

    ss >> std::get_time(&dt, date_time_format.c_str());

    std::time_t final_time;

    final_time = std::mktime(&dt);
    std::tm *ltm = localtime(&final_time);

    std::cout << "Year:" << 1900 + ltm->tm_year<<std::endl;
    std::cout << "Month: "<< 1 + ltm->tm_mon<< std::endl;
    std::cout << "Day: "<< ltm->tm_mday << std::endl;

    /*
    std::string date_time_format = "%Y-%m-%dT%H:%M:%S.000Z";
    std::istringstream ss{dtUtc};
    std::chrono::year_month_day date;

    ss >> std::chrono::parse(date_time_format, date);

    if (!ss) {
        return dtUtc;
    }

    std::cout << "Year: "  << date.year()  << '\n';
    std::cout << "Month: " << date.month() << '\n';
    std::cout << "Day: "   << date.day()   << '\n';
    */

    return dtUtc;
}

/**
 * Log FitGalgo API in.
 *
 * @param username Username for login.
 * @param password User's password for login.
 *
 * @return bool True if login was okay; false otherwise.
 */
bool login(std::string username, std::string password)
{
    try {
	HttpClient client(Context::host, Context::port);

	std::stringstream requestBody;
	requestBody << "username=" << username << "&password=" << password;
	client.addHeader("Content-Type", "application/x-www-form-urlencoded");
	client.addHeader(
	    "Content-Length",
	    std::to_string(requestBody.str().length()));
	std::string response = client.sendPostRequest("/auth/login", requestBody.str());

	size_t bodyStart = response.find("\r\n\r\n");
	if (bodyStart != std::string::npos && bodyStart + 4 < response.length()) {
	    std::string body = response.substr(bodyStart + 4);
	    rapidjson::Document document;
	    document.Parse(body.c_str());

	    static const char* kTypeNames[] = {
		"Null", "False", "True", "Object", "Array", "String", "Number"
	    };
	    for (rapidjson::Value::ConstMemberIterator itr = document.MemberBegin();
		 itr != document.MemberEnd(); ++itr) {
		// printf("Type of member %s is %s\n",
		//        itr->name.GetString(), kTypeNames[itr->value.GetType()]);
		if (std::string(itr->name.GetString()) == "access_token"
		    and std::string(kTypeNames[itr->value.GetType()]) == "String") {
		    Context::token = itr->value.GetString();
		    return true;
		}
	    }
	} else {
	    std::cerr << "Error: no se encontró el cuerpo en la respuesta." << std::endl;
	    return false;
	}
    } catch (const HttpClientException& e) {
	std::cerr << e.what() << std::endl;
	return false;
    } catch (const std::exception& e) {
	std::cerr << e.what() << std::endl;
	return false;
    }

    return false;
}

int main(int argc, char* argv[])
{
    if (argc < 5) {
	std::cerr << usage_help() << std::endl;
	return 1;
    }

    std::string host = "";
    std::string port = "";
    std::string* ptr_to_option = nullptr;

    std::for_each(argv + 1, argv + argc, [&ptr_to_option, &host, &port](char* arg) {
	if (strcmp(arg, "-h") == 0) {
	    ptr_to_option = &host;
	} else if (strcmp(arg, "-p") == 0) {
	    ptr_to_option = &port;
	} else if (ptr_to_option != nullptr) {
	    *ptr_to_option = arg;
	    ptr_to_option = nullptr;
	} else {
	    ptr_to_option = nullptr;
	}
    });

    if (host.empty() || port.empty()) {
	std::cerr << usage_help() << std::endl;
	return 1;
    }

    try {
	Context::host = host;
	Context::port = stoi(port);
    } catch (const std::invalid_argument& e) {
	std::cerr << "Invalid argument: port has to be a number: " << e.what() << std::endl;
	return 1;
    } catch (const std::out_of_range& e) {
	std::cerr << "Invalid argument: port out of range: " << e.what() << std::endl;
	return 1;
    }

    int option;
    std::string username;
    std::string password;
    do {
	system("clear");
	std::cout << Context::token << std::endl;
	std::cout << "MENU" << std::endl;
	std::cout << "-------------------------------------------" << std::endl;
	if (Context::token.empty()) {
	    std::cout << "1.- Login" << std::endl;
	} else {
	    std::cout << "1.- Logout" << std::endl;
	}
	std::cout << "2.- Upload file" << std::endl;
	std::cout << "3.- Steps" << std::endl;
	std::cout << "0.- Exit" << std::endl;
	std::cout << "Select an option: ";
	std::cin >> option;
	std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

	switch (option) {
	case 1:
	    if (Context::token.empty()) {
		std::cout << "Username: ";
		std::cin >> username;
		std::cout << "Password: ";
		disableEcho();
		std::cin >> password;
		enableEcho();
		std::cout << std::endl;
		std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
		if (login(username, password)) {
		    std::cout << "Login okay" << std::endl;
		} else {
		    std::cerr << "Login error" << std::endl;
		}
	    } else {
		Context::token = "";
		std::cout << "Logout";
	    }
	    std::cout << std::endl << "Press Enter to continue...";
	    std::cin.get();
	    break;
	case 2:
	    try {
		// Ask for file path until user response with a regular file.
		std::filesystem::path path;
		do {
		    std::cout << "File path: ";
		    std::cin >> path;
		    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
		} while (!std::filesystem::exists(path));

		if (std::filesystem::is_regular_file(path)) {
		    std::cout << "Send file " << path << " of "
			      << std::filesystem::file_size(path) << " bytes" << std::endl;
		    
		    // Open and read the file in a buffer.
		    std::ifstream file(path.string(), std::ios::binary);
		    std::stringstream buffer;
		    buffer << file.rdbuf();

		    // Send POST request with the file.
		    httplib::MultipartFormDataItems items = {
			{ "files", buffer.str(), path.filename().string(), "application/octet-stream" },
			{ "zone", "Europe/Madrid", "", "" }
		    };
		    httplib::Client client(Context::host, Context::port);
		    client.set_bearer_token_auth(Context::token);
	    
		    auto response = client.Post("/files/", items);
		    std::cout << response->status << std::endl;
		    std::cout << response->body << std::endl;
		} else if (std::filesystem::is_directory(path)) {
		    httplib::Client client(Context::host, Context::port);
		    client.set_bearer_token_auth(Context::token);
		    client.set_connection_timeout(0, 1000000); // 1000 milliseconds = 1 second
		    client.set_read_timeout(10, 0); // 10 seconds
		    client.set_write_timeout(10, 0); // 10 seconds

		    for (auto& e : std::filesystem::directory_iterator(path)) {
			std::cout << "Send file " << e.path() << " of "
				  << std::filesystem::file_size(e.path()) << " bytes" << std::endl;
		    
			// Open and read the file in a buffer.
			std::ifstream file(e.path().string(), std::ios::binary);
			std::stringstream buffer;
			buffer << file.rdbuf();
			file.close();

			// Send POST request with the file.
			httplib::MultipartFormDataItems items = {
			    { "files", buffer.str(), e.path().filename().string(), "application/octet-stream" },
			    { "zone", "Europe/Madrid", "", "" }
			};
		    
			auto response = client.Post("/files/", items);
			if (response != nullptr) {
			    std::cout << response->status << std::endl;
			    std::cout << response->body << std::endl;
			} else {
			    std::cout << "RESPONSE NULL" << std::endl;
			}
		    }
		}
	    } catch (std::filesystem::filesystem_error& error) {
		std::cerr << "EXCEPTION: " << error.what() << std::endl;
		std::cerr << "Prueba de nuevo" << std::endl;
	    }

	    std::cout << std::endl << "Press Enter to continue...";
	    std::cin.get();
	    break;
	case 3:
	    httplib::Client client(Context::host, Context::port);
	    client.set_bearer_token_auth(Context::token);
	    auto response = client.Get("/monitorings/steps");
	    if (response && (response->status >= 200 || response->status <=300)) {
		std::cout << "Status: " << response->status << std::endl;

		static const char* kTypeNames[] = {
		    "Null", "False", "True", "Object", "Array", "String", "Number"
		};

		std::map<std::string, int> steps {};

		rapidjson::Document document;
		document.Parse(response->body.c_str());
		std::cout << response->body << std::endl;
		for (auto& v : document.GetArray()) {
		    if (v.IsObject()) {
			rapidjson::Value::ConstMemberIterator itr_dt = v.FindMember("datetime_local");
			rapidjson::Value::ConstMemberIterator itr_steps = v.FindMember("total_steps");

			if (itr_dt != v.MemberEnd() && itr_steps != v.MemberEnd()) {
			    steps[std::string(itr_dt->value.GetString()).substr(0, 10)] = itr_steps->value.GetInt();
			}
			
			//rapidjson::Value::ConstMemberIterator itr_total_steps = v.FindMember("total_steps");
			//steps[itr_dt_utc->value.GetString()] = itr_total_steps->value.GetInt();
			
			
			/*for (auto& m : v.GetObject()) {
			    std::cout << "Type of member " << m.name.GetString()
				      << " is " << kTypeNames[m.value.GetType()]
				      << std::endl;
				      }*/
		    } else {
			std::cout << "No es un objeto" << std::endl;
		    }
		}

		for (auto& [k, v] : steps) {
		    std::cout << k << ": " << v << std::endl;
		}
	    } else {
		std::cout << "HTTP error: "
			  << httplib::to_string(response.error())
			  << std::endl;
	    }
	    
	    std::cout << std::endl << "Press Enter to continue...";
	    std::cin.get();
	    break;
	}
    } while (option != 0);

    return 0;
}
