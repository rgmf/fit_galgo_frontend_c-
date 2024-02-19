#ifndef _ES_RGMF_UI_TABULAR_H
#define _ES_RGMF_UI_TABULAR_H 1

#include <map>
#include <string>
#include <vector>
#include <iostream>

#include "colors.h"

using std::cout;
using std::endl;

namespace fitgalgo
{

const size_t COL_MAX_WIDTH = 20;

class Tabular
{
private:
    std::map<std::string, std::vector<std::string>> values;

    inline void print_header() const
    {
	cout << colors::BOLD << '|';
	for (const auto& value : values)
	{
	    cout << value.first;
	    for (size_t i = value.first.length(); i < COL_MAX_WIDTH; i++)
	    {
		cout << ' ';
	    }
	    cout << '|';
	}
	cout << colors::RESET << endl;
    }

    inline void print_separator() const
    {
	cout << '+';
	for (size_t i = 0; i < values.size(); i++)
	{
	    for (size_t j = 0; j < COL_MAX_WIDTH; j++)
	    {
		cout << "-";
	    }
	    cout << '+';
	}
	cout << endl;
    }

public:
    Tabular() : values{} {};
    Tabular(const std::vector<std::string>& headers)
    {
	values = std::map<std::string, std::vector<std::string>>();
	for (const auto& h : headers)
	    values[h] = std::vector<std::string>();
    }

    void add_header(const std::string& header)
    {
	values[header] = std::vector<std::string>();
    }

    void add_row(const std::vector<std::string>& row)
    {
	auto row_itr = row.begin();
	for (auto i = values.begin(); i != values.end() && row_itr != row.end(); i++, row_itr++)
	{
	    i->second.emplace_back(*row_itr);
	}
    }

    void add_value(const std::string& header, const std::string& value)
    {
	values[header].emplace_back(value);
    }

    void add_values(const std::string& header, const std::vector<std::string>& new_values)
    {
	values[header].insert(values[header].cend(), new_values.cbegin(), new_values.cend());
    }

    void print() const
    {
	bool has_items = true;
	size_t idx = 0;
	
	print_header();
	print_separator();

	while (has_items)
	{
	    has_items = false;

	    for (const auto& value : values)
	    {
		if (value.second.size() > idx)
		{
		    has_items = true;

		    cout << '|';
		    cout << value.second.at(idx);
		    for (size_t i = value.second.at(idx).length(); i < COL_MAX_WIDTH; i++)
		    {
			cout << ' ';
		    }
		}
	    }

	    if (has_items)
		cout << '|' << endl;

	    idx++;
	}

	print_separator();
    }
};

}

#endif // _ES_RGMF_UI_TABULAR_H
