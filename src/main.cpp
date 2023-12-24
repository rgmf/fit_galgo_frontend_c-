#include <iostream>
#include <string>
#include <filesystem>

#include <termios.h>

#include <rapidjson/document.h>
#include <httplib/httplib.h>

#include "core/api.h"
#include "core/stats.h"
#include "ui/shell.h"

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
inline void disableEcho()
{
    struct termios tty;
    tcgetattr(STDIN_FILENO, &tty);
    tty.c_lflag &= ~ECHO;
    tcsetattr(STDIN_FILENO, TCSANOW, &tty);
}

/**
 * Enables console characters impress.
 */
inline void enableEcho()
{
    struct termios tty;
    tcgetattr(STDIN_FILENO, &tty);
    tty.c_lflag |= ECHO;
    tcsetattr(STDIN_FILENO, TCSANOW, &tty);
}

int main(int argc, char* argv[])
{
    if (argc < 5)
    {
	std::cerr << usage_help() << std::endl;
	return 1;
    }

    std::string host = "";
    std::string port = "";
    int portInt;
    std::string* ptr_to_option = nullptr;

    std::for_each(argv + 1, argv + argc, [&ptr_to_option, &host, &port](char* arg)
    {
	if (strcmp(arg, "-h") == 0)
	{
	    ptr_to_option = &host;
	}
	else if (strcmp(arg, "-p") == 0)
	{
	    ptr_to_option = &port;
	}
	else if (ptr_to_option != nullptr)
	{
	    *ptr_to_option = arg;
	    ptr_to_option = nullptr;
	}
	else {
	    ptr_to_option = nullptr;
	}
    });

    if (host.empty() || port.empty())
    {
	std::cerr << usage_help() << std::endl;
	return 1;
    }

    try
    {
	portInt = stoi(port);
    }
    catch (const std::invalid_argument& e)
    {
	std::cerr << "Invalid argument: port has to be a number: " << e.what() << std::endl;
	return 1;
    }
    catch (const std::out_of_range& e)
    {
	std::cerr << "Invalid argument: port out of range: " << e.what() << std::endl;
	return 1;
    }

    fitgalgo::Connection conn(host, portInt);

    int option;
    std::string username;
    std::string password;
    do
    {
	system("clear");
	std::cout << "MENU" << std::endl;
	std::cout << "-------------------------------------------" << std::endl;
	if (!conn.has_token())
	    std::cout << "1.- Login" << std::endl;
	else
	    std::cout << "1.- Logout" << std::endl;
	std::cout << "2.- Upload file" << std::endl;
	std::cout << "3.- Steps" << std::endl;
	std::cout << "4.- Sleep" << std::endl;
	std::cout << "5.- Activities" << std::endl;
	std::cout << "0.- Exit" << std::endl;
	std::cout << "Select an option: ";
	std::cin >> option;
	std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

	switch (option)
	{
	case 1:
	    if (!conn.has_token())
	    {
		std::cout << "Username: ";
		std::cin >> username;
		std::cout << "Password: ";
		disableEcho();
		std::cin >> password;
		enableEcho();
		std::cout << std::endl;
		std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
		auto result = conn.login(username, password);
		if (result.is_valid())
		{
		    auto loginData = result.get_data();
		    std::cout << "Login okay" << std::endl;
		}
		else
		{
		    std::cerr << "Login error" << std::endl;
		    std::cerr << result.get_error().error_to_string() << std::endl;
		}
	    }
	    else
	    {
		conn.logout();
		std::cout << "Logout";
	    }
	    std::cout << std::endl << "Press Enter to continue...";
	    std::cin.get();
	    break;
	case 2:
	    try
	    {
		std::filesystem::path path;
		do
		{
		    std::cout << "File path: ";
		    std::cin >> path;
		    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
		} while (!std::filesystem::exists(path));

		auto results = conn.post_file(path);
		unsigned short accepted = 0;
		for (auto& result : results)
		{
		    if (result.is_valid())
		    {
			const auto& ufd = result.get_data();
			std::cout << "File: " << ufd.file_path
				  << " | accepted: " << ufd.accepted
				  << std::endl;
			if (!ufd.errors.empty())
			    std::cout << "Errors:" << std::endl;
			else
			    accepted++;

			for (auto& error : ufd.errors)
			{
			    std::cout << error << std::endl;
			}
		    }
		    else
		    {
			std::cerr << result.get_error().error_to_string() << std::endl;
		    }
		}
		std::cout << std::endl << "TOTAL: " << results.size() << std::endl;
		std::cout << "ACCEPTED: " << accepted << std::endl << std::endl;
	    }
	    catch (std::filesystem::filesystem_error& error)
	    {
		std::cerr << "EXCEPTION: " << error.what() << std::endl;
		std::cout << "Try again..." << std::endl;
	    }

	    std::cout << std::endl << "Press Enter to continue...";
	    std::cin.get();
	    break;
	case 3:
	    try
	    {
		auto result = conn.get_steps();
		if (result.is_valid())
		{
		    auto ui = fitgalgo::ShellSteps(result.get_data());
		    ui.loop();
		}
		else
		{
		    std::cerr << result.get_error().error_to_string() << std::endl;
		    std::cout << std::endl << "Press Enter to continue...";
		    std::cin.get();
		}
	    }
	    catch (const std::exception& e) {
		std::cerr << "Error: " << e.what() << std::endl;
		std::cout << std::endl << "Press Enter to continue...";
		std::cin.get();
	    }
	    break;
	case 4:
	    try
	    {
		auto result = conn.get_sleep();
		if (result.is_valid())
		{
		    auto ui = fitgalgo::ShellSleep(result.get_data());
		    ui.loop();
		}
		else
		{
		    std::cerr << result.get_error().error_to_string() << std::endl;
		    std::cout << std::endl << "Press Enter to continue...";
		    std::cin.get();
		}
	    }
	    catch (const std::exception& e) {
		std::cerr << "Error: " << e.what() << std::endl;
		std::cout << std::endl << "Press Enter to continue...";
		std::cin.get();
	    }
	    break;
	case 5:
	    try
	    {
		auto result = conn.get_activities();
		if (result.is_valid())
		{
		    auto ui = fitgalgo::ShellActivities(result.get_data());
		    ui.loop();
		}
		else
		{
		    std::cerr << result.get_error().error_to_string() << std::endl;
		    std::cout << std::endl << "Press Enter to continue...";
		    std::cin.get();
		}
	    }
	    catch (const std::exception& e) {
		std::cerr << "Error: " << e.what() << std::endl;
		std::cout << std::endl << "Press Enter to continue...";
		std::cin.get();
	    }
	    break;
	}
    } while (option != 0);

    return 0;
}
