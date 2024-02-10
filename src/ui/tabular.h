#ifndef _ES_RGMF_UI_TABULAR_H
#define _ES_RGMF_UI_TABULAR_H 1

#include <string>
#include <vector>
#include <iostream>

#include "colors.h"

using std::cout;
using std::endl;

namespace fitgalgo
{

const size_t COL_MAX_WIDTH = 20;

template <size_t N>
class Tabular
{
private:
    std::array<std::string, N> headers;
    std::vector<std::array<std::string, N>> values;

    inline void print_header() const
    {
	cout << colors::BOLD << '|';
	for (const auto& h : headers)
	{
	    cout << h;
	    for (size_t i = h.length(); i < COL_MAX_WIDTH; i++)
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
	for (size_t i = 0; i < headers.size(); i++)
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
    Tabular() : headers{}, values{} {};
    Tabular(const std::array<std::string, N>& headers)
	: headers(headers), values() {};

    void append_row(const std::array<std::string, N>& row)
    {
	values.emplace_back(row);
    }
    
    void print() const
    {
	print_header();
	print_separator();
	
	for (const auto& row : values)
	{
	    cout << '|';
	    for (const auto& v : row)
	    {
		cout << v;
		for (size_t i = v.length(); i < COL_MAX_WIDTH; i++)
		{
		    cout << ' ';
		}
		cout << '|';
	    }
	    cout << endl;

	    print_separator();
	}
    }
};

}

#endif // _ES_RGMF_UI_TABULAR_H
