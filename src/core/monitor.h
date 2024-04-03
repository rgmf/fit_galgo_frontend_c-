#ifndef ES_RGMF_CORE_MONITOR_H
#define ES_RGMF_CORE_MONITOR_H 1

#include <vector>
#include <string>

#include "rapidjson/document.h"
#include "rapidjson/encodings.h"

#include "definitions.h"

using std::string;
using rapidjson_value = rapidjson::GenericValue<rapidjson::UTF8<>>;

namespace fitgalgo
{

class monitor
{
private:
    std::vector<string> field_errors_;
    
    string username_{};
    string datetime_utc_{};
    string datetime_local_{};
    ushort total_steps_{};
    float total_distance_{};
    float total_calories_{};
    string zone_info_{};

public:
    monitor(const rapidjson_value& v);

    bool ok() const;
    error_t error() const;
};

}

#endif // ES_RGMF_CORE_MONITOR_H
