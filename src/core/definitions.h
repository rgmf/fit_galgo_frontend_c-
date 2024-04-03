#ifndef ES_RGMF_CORE_DEFINITIONS_H
#define ES_RGMF_CORE_DEFINITIONS_H 1

#include <string>

using std::string;

namespace fitgalgo
{

enum error_code_e
{
    OK=0,
    JSON
};

struct error_t
{
    error_code_e code;
    string message;
};

} // namespace fitgalgo

#endif // ES_RGMF_CORE_DEFINITIONS_H
