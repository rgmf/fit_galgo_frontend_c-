#ifndef ES_RGMF_CORE_API_H
#define ES_RGMF_CORE_API_H 1

#include <string>
#include <vector>

#include "rapidjson/document.h"

#include "definitions.h"

using std::string;

namespace fitgalgo
{

template <typename T>
class json_decoder
{
private:
    std::vector<error_t> errors_;
    std::vector<T> data_;

public:
    json_decoder() = delete;
    explicit json_decoder(const rapidjson::Document& doc);
};

} // namespace fitgalgo

#endif // ES_RGMF_CORE_API_H
