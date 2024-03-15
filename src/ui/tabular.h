#include <utility>
#ifndef _ES_RGMF_UI_TABULAR_H
#define _ES_RGMF_UI_TABULAR_H 1

#include <map>
#include <string>
#include <vector>
#include <iostream>

#include "../utils/string.h"
#include "colors.h"

using std::cout;
using std::endl;

namespace fitgalgo
{

const size_t COL_MIN_WIDTH = 20;

class Tabular
{
private:
    std::map<std::string, std::vector<std::string>> values;
    std::vector<std::string> insertion_order;

    inline void print_header(const size_t& w) const
    {
	cout << colors::BOLD << '|';
	for (const auto& header : insertion_order)
	{
	    cout << ' ' << header << ' ';
	    for (size_t i = fitgalgo::mb_strlen(header) + 2; i < w; i++)
	    {
		cout << ' ';
	    }
	    cout << '|';
	}
	cout << colors::RESET << endl;
    }

    inline void print_separator(const size_t& w) const
    {
	cout << '+';
	for (size_t i = 0; i < values.size(); i++)
	{
	    for (size_t j = 0; j < w; j++)
	    {
		cout << '-';
	    }
	    cout << '+';
	}
	cout << endl;
    }

public:
    Tabular() : values{}, insertion_order{} {};
    Tabular(const std::vector<std::string>& headers)
    {
	values = std::map<std::string, std::vector<std::string>>();
	for (const auto& h : headers)
	{
	    insertion_order.emplace_back(h);
	    values[h] = std::vector<std::string>();
	}
    }

    void add_header(const std::string& header)
    {
	insertion_order.emplace_back(header);
	values[header] = std::vector<std::string>();
    }

    void add_row(const std::vector<std::pair<std::string, std::string>>& row)
    {
	for (const auto& [k, v] : row)
	    values[k].emplace_back(v);
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
	size_t width = COL_MIN_WIDTH;

	for (const auto& [h, v] : values)
	    for (const auto& s : v)
		width = fitgalgo::mb_strlen(s) + 2 > width ? fitgalgo::mb_strlen(s) + 2 : width;

	print_header(width);
	print_separator(width);

	while (has_items)
	{
	    has_items = false;

	    for (const auto& k : insertion_order)
	    {
		const auto& value = values.at(k);
		if (value.size() > idx)
		{
		    has_items = true;

		    cout << '|'
			 << ' ' << value.at(idx) << ' ';
		    for (size_t i = fitgalgo::mb_strlen(value.at(idx)) + 2; i < width; i++)
		    {
			cout << ' ';
		    }
		}
		else
		{
		    cout << '|';
		    for (size_t i = 0; i < width; i++)
			cout << ' ';
		}
	    }

	    cout << '|' << endl;

	    idx++;
	}

	print_separator(width);
    }
};

}

#endif // _ES_RGMF_UI_TABULAR_H
