#include <catch2/catch_test_macros.hpp>

#include "rapidjson/document.h"

#include "../src/core/definitions.h"
#include "../src/core/monitor.h"

TEST_CASE("Core Monitor: JSON ok", "[core_monitor]")
{
    std::string json = R"({
      "username": "roman",
      "datetime_utc": "2024-03-28T18:31:00Z",
      "total_steps": 12762,
      "total_distance": 13890.25,
      "total_calories": 2636,
      "zone_info": "Europe/Madrid",
      "datetime_local": "2024-03-28T19:31:00+01:00"
    })";

    rapidjson::Document document;
    document.Parse(json.c_str());

    fitgalgo::monitor m{document.GetObject()};

    REQUIRE(m.ok());
    REQUIRE(m.error().code == fitgalgo::error_code_e::OK);
    REQUIRE(m.error().message.empty());
}

TEST_CASE("Core Monitor: JSON ok with minimal fields", "[core_monitor]")
{
    std::string json = R"({
      "username": "roman",
      "datetime_utc": "2024-03-28T18:31:00Z",
      "datetime_local": "2024-03-28T19:31:00+01:00"
    })";

    rapidjson::Document document;
    document.Parse(json.c_str());

    fitgalgo::monitor m{document.GetObject()};

    REQUIRE(m.ok());
    REQUIRE(m.error().code == fitgalgo::error_code_e::OK);
    REQUIRE(m.error().message.empty());
}

TEST_CASE("Core Monitor: JSON error: username is needed", "[core_monitor]")
{
    std::string json = R"({
      "datetime_utc": "2024-03-28T18:31:00Z",
      "total_steps": 12762,
      "total_distance": 13890.25,
      "total_calories": 2636,
      "zone_info": "Europe/Madrid",
      "datetime_local": "2024-03-28T19:31:00+01:00"
    })";

    rapidjson::Document document;
    document.Parse(json.c_str());

    fitgalgo::monitor m{document.GetObject()};

    REQUIRE(!m.ok());
    REQUIRE(m.error().code == fitgalgo::error_code_e::JSON);
    REQUIRE(m.error().message.find("'username'") != std::string::npos);
}

TEST_CASE("Core Monitor: JSON error: datetime_utc is needed", "[core_monitor]")
{
    std::string json = R"({
      "username": "roman",
      "total_steps": 12762,
      "total_distance": 13890.25,
      "total_calories": 2636,
      "zone_info": "Europe/Madrid",
      "datetime_local": "2024-03-28T19:31:00+01:00"
    })";

    rapidjson::Document document;
    document.Parse(json.c_str());

    fitgalgo::monitor m{document.GetObject()};

    REQUIRE(!m.ok());
    REQUIRE(m.error().code == fitgalgo::error_code_e::JSON);
    REQUIRE(m.error().message.find("'datetime_utc'") != std::string::npos);
}

TEST_CASE("Core Monitor: JSON error: datetime_local is needed", "[core_monitor]")
{
    std::string json = R"({
      "username": "roman",
      "datetime_utc": "2024-03-28T18:31:00Z",
      "total_steps": 12762,
      "total_distance": 13890.25,
      "total_calories": 2636,
      "zone_info": "Europe/Madrid"
    })";

    rapidjson::Document document;
    document.Parse(json.c_str());

    fitgalgo::monitor m{document.GetObject()};

    REQUIRE(!m.ok());
    REQUIRE(m.error().code == fitgalgo::error_code_e::JSON);
    REQUIRE(m.error().message.find("'datetime_local'") != std::string::npos);
}

TEST_CASE("Core Monitor: JSON error: datetimes are needed", "[core_monitor]")
{
    std::string json = R"({
      "username": "roman",
      "total_steps": 12762,
      "total_distance": 13890.25,
      "total_calories": 2636,
      "zone_info": "Europe/Madrid"
    })";

    rapidjson::Document document;
    document.Parse(json.c_str());

    fitgalgo::monitor m{document.GetObject()};

    REQUIRE(!m.ok());
    REQUIRE(m.error().code == fitgalgo::error_code_e::JSON);
    REQUIRE(m.error().message.find("'datetime_utc'") != std::string::npos);
    REQUIRE(m.error().message.find("'datetime_local'") != std::string::npos);
}

TEST_CASE("Core Monitor: JSON error: all fields neede are not there", "[core_monitor]")
{
    std::string json = R"({
      "total_steps": 12762,
      "total_distance": 13890.25,
      "total_calories": 2636,
      "zone_info": "Europe/Madrid"
    })";

    rapidjson::Document document;
    document.Parse(json.c_str());

    fitgalgo::monitor m{document.GetObject()};

    REQUIRE(!m.ok());
    REQUIRE(m.error().code == fitgalgo::error_code_e::JSON);
    REQUIRE(m.error().message.find("'username'") != std::string::npos);
    REQUIRE(m.error().message.find("'datetime_utc'") != std::string::npos);
    REQUIRE(m.error().message.find("'datetime_local'") != std::string::npos);
}
