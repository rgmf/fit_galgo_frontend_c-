#include <sstream>

#include "monitor.h"
#include "definitions.h"

namespace fitgalgo
{

monitor::monitor(const rapidjson_value& v) :
    field_errors_{}
{
    auto i = v.FindMember("username");
    if (i != v.MemberEnd() && i->value.IsString())
	username_ = i->value.GetString();
    else
	field_errors_.emplace_back("username");

    i = v.FindMember("datetime_utc");
    if (i != v.MemberEnd() && i->value.IsString())
	datetime_utc_ = i->value.GetString();
    else
	field_errors_.emplace_back("datetime_utc");

    i = v.FindMember("datetime_local");
    if (i != v.MemberEnd() && i->value.IsString())
	datetime_local_ = i->value.GetString();
    else
	field_errors_.emplace_back("datetime_local");

    i = v.FindMember("total_steps");
    if (i != v.MemberEnd() && i->value.IsInt())
	total_steps_ = i->value.GetInt();

    i = v.FindMember("total_distance");
    if (i != v.MemberEnd() && i->value.IsFloat())
	total_distance_ = i->value.GetFloat();

    i = v.FindMember("total_calories");
    if (i != v.MemberEnd() && i->value.IsFloat())
	total_calories_ = i->value.GetFloat();

    i = v.FindMember("zone_info");
    if (i != v.MemberEnd() && i->value.IsString())
	zone_info_ = i->value.GetString();
}

bool monitor::ok() const
{
    return field_errors_.empty();
}

error_t monitor::error() const
{
    const size_t count = field_errors_.size();
    if (count == 0)
	return error_t{.code=error_code_e::OK, .message=""};
    else if (count == 1)
	return error_t{
	    .code=error_code_e::JSON,
	    .message="the field '" + field_errors_[0] + "' is needed"};
    else
    {
	std::stringstream ss;
	ss << "the following fields are needed: '" << field_errors_[0] << "'";
	for (size_t i = 0; i < count - 1; ++i)
	{
	    ss << ", '" << field_errors_[i] << "'";
	}
	ss << " and '" << field_errors_[count - 1] << "'";
	return error_t{.code=error_code_e::JSON, .message=ss.str()};
    }
}

} // namespace fitgalgo
