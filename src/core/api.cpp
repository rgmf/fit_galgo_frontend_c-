#include "api.h"
#include "definitions.h"

namespace fitgalgo
{

template <typename T>
json_decoder<T>::json_decoder(const rapidjson::Document& doc) :
    errors_{},
    data_{}
{
    const auto& data = doc.FindMember("data");
    const auto& count = doc.FindMember("count");
    if (data == doc.MemberEnd() || !data->value.IsArray() ||
	count == doc.MemberEnd() || !count->value.IsInt())
    {
	errors_.emplace_back(error_t{.code = error_code_e::JSON,
			     .message = "expected 'data' array and 'count' value in the JSON"});
	return;
    }

    for (const auto& i : data->value.GetArray())
    {
	T v{i};
	if (v.ok())
	    data_.emplace_back(v);
	else
	    errors_.emplace_back(v.error());
    }
}

} // namespace fitgalgo
