#include <iostream>
#include <string>

#include <termios.h>

#include <rapidjson/document.h>
#include <httplib/httplib.h>

#include "api/fitgalgo.h"

/**
 * @return Message with information and help about programm usage.
 */
inline std::string usage_help()
{
    return "Usage: fitgalgo -h [host] -p [port]";
}

/**
 * Disables console characters impress.
 */
inline void disableEcho() {
    struct termios tty;
    tcgetattr(STDIN_FILENO, &tty);
    tty.c_lflag &= ~ECHO;
    tcsetattr(STDIN_FILENO, TCSANOW, &tty);
}

/**
 * Enables console characters impress.
 */
inline void enableEcho() {
    struct termios tty;
    tcgetattr(STDIN_FILENO, &tty);
    tty.c_lflag |= ECHO;
    tcsetattr(STDIN_FILENO, TCSANOW, &tty);
}

int main(int argc, char* argv[])
{
    if (argc < 5) {
	std::cerr << usage_help() << std::endl;
	return 1;
    }

    std::string host = "";
    std::string port = "";
    int portInt;
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
	portInt = stoi(port);
    } catch (const std::invalid_argument& e) {
	std::cerr << "Invalid argument: port has to be a number: " << e.what() << std::endl;
	return 1;
    } catch (const std::out_of_range& e) {
	std::cerr << "Invalid argument: port out of range: " << e.what() << std::endl;
	return 1;
    }

    fitgalgo::Connection fitgalgoConn(host, portInt);

    int option;
    std::string username;
    std::string password;
    do {
	system("clear");
	std::cout << "MENU" << std::endl;
	std::cout << "-------------------------------------------" << std::endl;
	if (!fitgalgoConn.hasToken()) {
	    std::cout << "1.- Login" << std::endl;
	} else {
	    std::cout << "1.- Logout" << std::endl;
	}
	std::cout << "2.- Upload file" << std::endl;
	std::cout << "3.- Steps" << std::endl;
	std::cout << "4.- Sleep" << std::endl;
	std::cout << "0.- Exit" << std::endl;
	std::cout << "Select an option: ";
	std::cin >> option;
	std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

	switch (option) {
	case 1:
	    if (!fitgalgoConn.hasToken()) {
		std::cout << "Username: ";
		std::cin >> username;
		std::cout << "Password: ";
		disableEcho();
		std::cin >> password;
		enableEcho();
		std::cout << std::endl;
		std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
		auto result = fitgalgoConn.login(username, password);
		if (result.isValid()) {
		    auto loginData = result.getData();
		    std::cout << "Login okay" << std::endl;
		} else {
		    std::cerr << "Login error" << std::endl;
		    std::cerr << result.getError().errorToString() << std::endl;
		}
	    } else {
		fitgalgoConn.logout();
		std::cout << "Logout";
	    }
	    std::cout << std::endl << "Press Enter to continue...";
	    std::cin.get();
	    break;
	case 2:
	    try {
		std::filesystem::path path;
		do {
		    std::cout << "File path: ";
		    std::cin >> path;
		    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
		} while (!std::filesystem::exists(path));

		auto resultList = fitgalgoConn.postFile(path);
		for (auto& result : resultList) {
		    if (result.isValid()) {
			const auto& ufd = result.getData();
			std::cout << "File: " << ufd.filePath
				  << " | accepted: " << ufd.accepted
				  << std::endl;
			if (!ufd.errors.empty()) {
			    std::cout << "Errors:" << std::endl;
			}
			for (auto& error : ufd.errors) {
			    std::cout << error << std::endl;
			}
		    } else {
			std::cerr << result.getError().errorToString() << std::endl;
		    }
		}
	    } catch (std::filesystem::filesystem_error& error) {
		std::cerr << "EXCEPTION: " << error.what() << std::endl;
		std::cout << "Try again..." << std::endl;
	    }

	    std::cout << std::endl << "Press Enter to continue...";
	    std::cin.get();
	    break;
	case 3:
	    try {
		auto result = fitgalgoConn.getSteps();
		if (result.isValid()) {
		    for (auto& [k, v] : result.getData().steps) {
			std::cout << k << ": "
				  << v.steps << " | "
				  << v.distance << " | "
				  << v.calories
				  << std::endl;
		    }
		} else {
		    std::cerr << result.getError().errorToString() << std::endl;
		}
	    } catch (const std::exception& e) {
		std::cerr << "Error: " << e.what() << std::endl;
	    }

	    std::cout << std::endl << "Press Enter to continue...";
	    std::cin.get();
	    break;
	case 4:
	    try {
		auto result = fitgalgoConn.getSleep();
		if (result.isValid()) {
		    for (auto& [k, v] : result.getData().sleep) {
			std::cout << k << ": "
				  << "Overall sleep score: "
				  << v.assessment.overall_sleep_score << " | "
				  << "Sleep quality score: "
				  << v.assessment.sleep_quality_score << " | "
				  << "Awakenings count: "
				  << v.assessment.awakenings_count << " | "
				  << v.assessment.combined_awake_score << " | "
				  << v.assessment.awake_time_score << " | "
				  << v.assessment.awakenings_count_score << " | "
				  << v.assessment.deep_sleep_score << " | "
				  << v.assessment.sleep_duration_score << " | "
				  << v.assessment.light_sleep_score << " | "
				  << v.assessment.sleep_recovery_score << " | "
				  << v.assessment.rem_sleep_score << " | "
				  << v.assessment.sleep_restlessness_score << " | "
				  << v.assessment.interruptions_score << " | "
				  << v.assessment.average_stress_during_sleep
				  << std::endl;
			std::cout << "Number of levels: " << v.levels.size() << std::endl << std::endl;
		    }
		} else {
		    std::cerr << result.getError().errorToString() << std::endl;
		}
	    } catch (const std::exception& e) {
		std::cerr << "Error: " << e.what() << std::endl;
	    }

	    std::cout << std::endl << "Press Enter to continue...";
	    std::cin.get();
	    break;
	}
    } while (option != 0);

    return 0;
}
