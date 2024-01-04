#include <iostream>
#include <string>

#include "ui/shell.h"

/**
 * @return Message with information and help about programm usage.
 */
inline std::string usage_help()
{
    return "Usage: fitgalgo -h [host] -p [port]";
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
    ushort portInt;
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

    fitgalgo::Shell shell{host, portInt};
    shell.loop();

    return 0;
}
