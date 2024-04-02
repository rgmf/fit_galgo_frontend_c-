#include <catch2/catch_test_macros.hpp>

#include "../src/core/date.h"

using fitgalgo::date::date_type_e;
using fitgalgo::date::iso_date;

TEST_CASE("Core Date: empty string", "[core_date]")
{
    iso_date d{""};
    REQUIRE(d.date_type() == date_type_e::INVALID);
    REQUIRE(d.value() == "");
    REQUIRE(d.ymd().ok() == false);
    REQUIRE(d.ymd().year() == chrono_year{0});
    REQUIRE(d.ymd().month() == chrono_month{0});
    REQUIRE(d.ymd().day() == chrono_day{0});
    REQUIRE(d.date().year == 0);
    REQUIRE(d.date().month == 0);
    REQUIRE(d.date().day == 0);
    REQUIRE(d.time().hour == 0);
    REQUIRE(d.time().minutes == 0);
    REQUIRE(d.time().seconds == 0);
    REQUIRE(d.time().millis == 0);
    REQUIRE(d.zone_info().hours == 0);
    REQUIRE(d.zone_info().minutes == 0);
}

TEST_CASE("Core Date: invalid string: less than 10 chars", "[core_date]")
{
    iso_date d{"20201010"};
    REQUIRE(d.date_type() == date_type_e::INVALID);
    REQUIRE(d.value() == "20201010");
    REQUIRE(d.ymd().ok() == false);
    REQUIRE(d.ymd().year() == chrono_year{0});
    REQUIRE(d.ymd().month() == chrono_month{0});
    REQUIRE(d.ymd().day() == chrono_day{0});
    REQUIRE(d.date().year == 0);
    REQUIRE(d.date().month == 0);
    REQUIRE(d.date().day == 0);
    REQUIRE(d.time().hour == 0);
    REQUIRE(d.time().minutes == 0);
    REQUIRE(d.time().seconds == 0);
    REQUIRE(d.time().millis == 0);
    REQUIRE(d.zone_info().hours == 0);
    REQUIRE(d.zone_info().minutes == 0);
}

TEST_CASE("Core Date: invalid string: 10 chars", "[core_date]")
{
    iso_date d{"asdfghjklñ"};
    REQUIRE(d.date_type() == date_type_e::INVALID);
    REQUIRE(d.value() == "asdfghjklñ");
    REQUIRE(d.ymd().ok() == false);
    REQUIRE(d.ymd().year() == chrono_year{0});
    REQUIRE(d.ymd().month() == chrono_month{0});
    REQUIRE(d.ymd().day() == chrono_day{0});
    REQUIRE(d.date().year == 0);
    REQUIRE(d.date().month == 0);
    REQUIRE(d.date().day == 0);
    REQUIRE(d.time().hour == 0);
    REQUIRE(d.time().minutes == 0);
    REQUIRE(d.time().seconds == 0);
    REQUIRE(d.time().millis == 0);
    REQUIRE(d.zone_info().hours == 0);
    REQUIRE(d.zone_info().minutes == 0);
}

TEST_CASE("Core Date: invalid iso_date day", "[core_date]")
{
    iso_date d{"2020-01-32"};
    REQUIRE(d.date_type() == date_type_e::INVALID);
    REQUIRE(d.value() == "2020-01-32");
    REQUIRE(d.ymd().ok() == false);
    REQUIRE(d.ymd().year() == chrono_year{0});
    REQUIRE(d.ymd().month() == chrono_month{0});
    REQUIRE(d.ymd().day() == chrono_day{0});
    REQUIRE(d.date().year == 2020);
    REQUIRE(d.date().month == 1);
    REQUIRE(d.date().day == 32);
    REQUIRE(d.time().hour == 0);
    REQUIRE(d.time().minutes == 0);
    REQUIRE(d.time().seconds == 0);
    REQUIRE(d.time().millis == 0);
    REQUIRE(d.zone_info().hours == 0);
    REQUIRE(d.zone_info().minutes == 0);
}

TEST_CASE("Core Date: invalid date month", "[core_date]")
{
    iso_date d{"2020-00-01"};
    REQUIRE(d.date_type() == date_type_e::INVALID);
    REQUIRE(d.value() == "2020-00-01");
    REQUIRE(d.ymd().ok() == false);
    REQUIRE(d.ymd().year() == chrono_year{0});
    REQUIRE(d.ymd().month() == chrono_month{0});
    REQUIRE(d.ymd().day() == chrono_day{0});
    REQUIRE(d.date().year == 2020);
    REQUIRE(d.date().month == 0);
    REQUIRE(d.date().day == 1);
    REQUIRE(d.time().hour == 0);
    REQUIRE(d.time().minutes == 0);
    REQUIRE(d.time().seconds == 0);
    REQUIRE(d.time().millis == 0);
    REQUIRE(d.zone_info().hours == 0);
    REQUIRE(d.zone_info().minutes == 0);
}

TEST_CASE("Core Date: invalid no leap date", "[core_date]")
{
    iso_date d{"2019-02-29"};
    REQUIRE(d.date_type() == date_type_e::INVALID);
    REQUIRE(d.value() == "2019-02-29");
    REQUIRE(d.ymd().ok() == false);
    REQUIRE(d.ymd().year() == chrono_year{0});
    REQUIRE(d.ymd().month() == chrono_month{0});
    REQUIRE(d.ymd().day() == chrono_day{0});
    REQUIRE(d.date().year == 2019);
    REQUIRE(d.date().month == 2);
    REQUIRE(d.date().day == 29);
    REQUIRE(d.time().hour == 0);
    REQUIRE(d.time().minutes == 0);
    REQUIRE(d.time().seconds == 0);
    REQUIRE(d.time().millis == 0);
    REQUIRE(d.zone_info().hours == 0);
    REQUIRE(d.zone_info().minutes == 0);
}

TEST_CASE("Core Date: invalid leap date", "[core_date]")
{
    iso_date d{"2024-02-30"};
    REQUIRE(d.date_type() == date_type_e::INVALID);
    REQUIRE(d.value() == "2024-02-30");
    REQUIRE(d.ymd().ok() == false);
    REQUIRE(d.ymd().year() == chrono_year{0});
    REQUIRE(d.ymd().month() == chrono_month{0});
    REQUIRE(d.ymd().day() == chrono_day{0});
    REQUIRE(d.date().year == 2024);
    REQUIRE(d.date().month == 2);
    REQUIRE(d.date().day == 30);
    REQUIRE(d.time().hour == 0);
    REQUIRE(d.time().minutes == 0);
    REQUIRE(d.time().seconds == 0);
    REQUIRE(d.time().millis == 0);
    REQUIRE(d.zone_info().hours == 0);
    REQUIRE(d.zone_info().minutes == 0);
}

TEST_CASE("Core Date: invalid string: less than 19 chars", "[core_date]")
{
    iso_date d{"2020-10-10T00:00:0"};
    REQUIRE(d.date_type() == date_type_e::INVALID);
    REQUIRE(d.value() == "2020-10-10T00:00:0");
    REQUIRE(d.ymd().ok() == false);
    REQUIRE(d.ymd().year() == chrono_year{0});
    REQUIRE(d.ymd().month() == chrono_month{0});
    REQUIRE(d.ymd().day() == chrono_day{0});
    REQUIRE(d.date().year == 0);
    REQUIRE(d.date().month == 0);
    REQUIRE(d.date().day == 0);
    REQUIRE(d.time().hour == 0);
    REQUIRE(d.time().minutes == 0);
    REQUIRE(d.time().seconds == 0);
    REQUIRE(d.time().millis == 0);
    REQUIRE(d.zone_info().hours == 0);
    REQUIRE(d.zone_info().minutes == 0);
}

TEST_CASE("Core Date: invalid string: 19 chars", "[core_date]")
{
    iso_date d{"2020-20-20T00:00:00"};
    REQUIRE(d.date_type() == date_type_e::INVALID);
    REQUIRE(d.value() == "2020-20-20T00:00:00");
    REQUIRE(d.ymd().ok() == false);
    REQUIRE(d.ymd().year() == chrono_year{0});
    REQUIRE(d.ymd().month() == chrono_month{0});
    REQUIRE(d.ymd().day() == chrono_day{0});
    REQUIRE(d.date().year == 2020);
    REQUIRE(d.date().month == 20);
    REQUIRE(d.date().day == 20);
    REQUIRE(d.time().hour == 0);
    REQUIRE(d.time().minutes == 0);
    REQUIRE(d.time().seconds == 0);
    REQUIRE(d.time().millis == 0);
    REQUIRE(d.zone_info().hours == 0);
    REQUIRE(d.zone_info().minutes == 0);
}

TEST_CASE("Core Date: invalid datetime day", "[core_date]")
{
    iso_date d{"2020-01-32T00:00:00"};
    REQUIRE(d.date_type() == date_type_e::INVALID);
    REQUIRE(d.value() == "2020-01-32T00:00:00");
    REQUIRE(d.ymd().ok() == false);
    REQUIRE(d.ymd().year() == chrono_year{0});
    REQUIRE(d.ymd().month() == chrono_month{0});
    REQUIRE(d.ymd().day() == chrono_day{0});
    REQUIRE(d.date().year == 2020);
    REQUIRE(d.date().month == 1);
    REQUIRE(d.date().day == 32);
    REQUIRE(d.time().hour == 0);
    REQUIRE(d.time().minutes == 0);
    REQUIRE(d.time().seconds == 0);
    REQUIRE(d.time().millis == 0);
    REQUIRE(d.zone_info().hours == 0);
    REQUIRE(d.zone_info().minutes == 0);
}

TEST_CASE("Core Date: invalid datetime month", "[core_date]")
{
    iso_date d{"2020-14-10T00:00:00"};
    REQUIRE(d.date_type() == date_type_e::INVALID);
    REQUIRE(d.value() == "2020-14-10T00:00:00");
    REQUIRE(d.ymd().ok() == false);
    REQUIRE(d.ymd().year() == chrono_year{0});
    REQUIRE(d.ymd().month() == chrono_month{0});
    REQUIRE(d.ymd().day() == chrono_day{0});
    REQUIRE(d.date().year == 2020);
    REQUIRE(d.date().month == 14);
    REQUIRE(d.date().day == 10);
    REQUIRE(d.time().hour == 0);
    REQUIRE(d.time().minutes == 0);
    REQUIRE(d.time().seconds == 0);
    REQUIRE(d.time().millis == 0);
    REQUIRE(d.zone_info().hours == 0);
    REQUIRE(d.zone_info().minutes == 0);
}

TEST_CASE("Core Date: invalid no leap datetime", "[core_date]")
{
    iso_date d{"2019-02-29T00:00:00"};
    REQUIRE(d.date_type() == date_type_e::INVALID);
    REQUIRE(d.value() == "2019-02-29T00:00:00");
    REQUIRE(d.ymd().ok() == false);
    REQUIRE(d.ymd().year() == chrono_year{0});
    REQUIRE(d.ymd().month() == chrono_month{0});
    REQUIRE(d.ymd().day() == chrono_day{0});
    REQUIRE(d.date().year == 2019);
    REQUIRE(d.date().month == 2);
    REQUIRE(d.date().day == 29);
    REQUIRE(d.time().hour == 0);
    REQUIRE(d.time().minutes == 0);
    REQUIRE(d.time().seconds == 0);
    REQUIRE(d.time().millis == 0);
    REQUIRE(d.zone_info().hours == 0);
    REQUIRE(d.zone_info().minutes == 0);
}

TEST_CASE("Core Date: invalid leap datetime", "[core_date]")
{
    iso_date d{"2024-02-30T00:00:00"};
    REQUIRE(d.date_type() == date_type_e::INVALID);
    REQUIRE(d.value() == "2024-02-30T00:00:00");
    REQUIRE(d.ymd().ok() == false);
    REQUIRE(d.ymd().year() == chrono_year{0});
    REQUIRE(d.ymd().month() == chrono_month{0});
    REQUIRE(d.ymd().day() == chrono_day{0});
    REQUIRE(d.date().year == 2024);
    REQUIRE(d.date().month == 2);
    REQUIRE(d.date().day == 30);
    REQUIRE(d.time().hour == 0);
    REQUIRE(d.time().minutes == 0);
    REQUIRE(d.time().seconds == 0);
    REQUIRE(d.time().millis == 0);
    REQUIRE(d.zone_info().hours == 0);
    REQUIRE(d.zone_info().minutes == 0);
}

TEST_CASE("Core Date: invalid datetime hour", "[core_date]")
{
    iso_date d{"2020-03-31T25:00:00"};
    REQUIRE(d.date_type() == date_type_e::INVALID);
    REQUIRE(d.value() == "2020-03-31T25:00:00");
    REQUIRE(d.ymd().ok() == false);
    REQUIRE(d.ymd().year() == chrono_year{0});
    REQUIRE(d.ymd().month() == chrono_month{0});
    REQUIRE(d.ymd().day() == chrono_day{0});
    REQUIRE(d.date().year == 2020);
    REQUIRE(d.date().month == 3);
    REQUIRE(d.date().day == 31);
    REQUIRE(d.time().hour == 25);
    REQUIRE(d.time().minutes == 0);
    REQUIRE(d.time().seconds == 0);
    REQUIRE(d.time().millis == 0);
    REQUIRE(d.zone_info().hours == 0);
    REQUIRE(d.zone_info().minutes == 0);
}

TEST_CASE("Core Date: invalid datetime minutes", "[core_date]")
{
    iso_date d{"2020-04-30T22:60:00.999"};
    REQUIRE(d.date_type() == date_type_e::INVALID);
    REQUIRE(d.value() == "2020-04-30T22:60:00.999");
    REQUIRE(d.ymd().ok() == false);
    REQUIRE(d.ymd().year() == chrono_year{0});
    REQUIRE(d.ymd().month() == chrono_month{0});
    REQUIRE(d.ymd().day() == chrono_day{0});
    REQUIRE(d.date().year == 2020);
    REQUIRE(d.date().month == 4);
    REQUIRE(d.date().day == 30);
    REQUIRE(d.time().hour == 22);
    REQUIRE(d.time().minutes == 60);
    REQUIRE(d.time().seconds == 0);
    REQUIRE(d.time().millis == 999);
    REQUIRE(d.zone_info().hours == 0);
    REQUIRE(d.zone_info().minutes == 0);
}

TEST_CASE("Core Date: invalid datetime seconds", "[core_date]")
{
    iso_date d{"2020-04-30T22:37:60"};
    REQUIRE(d.date_type() == date_type_e::INVALID);
    REQUIRE(d.value() == "2020-04-30T22:37:60");
    REQUIRE(d.ymd().ok() == false);
    REQUIRE(d.ymd().year() == chrono_year{0});
    REQUIRE(d.ymd().month() == chrono_month{0});
    REQUIRE(d.ymd().day() == chrono_day{0});
    REQUIRE(d.date().year == 2020);
    REQUIRE(d.date().month == 4);
    REQUIRE(d.date().day == 30);
    REQUIRE(d.time().hour == 22);
    REQUIRE(d.time().minutes == 37);
    REQUIRE(d.time().seconds == 60);
    REQUIRE(d.zone_info().hours == 0);
    REQUIRE(d.zone_info().minutes == 0);
}

TEST_CASE("Core Date: no iso8601 date", "[core_date]")
{
    iso_date d{"2020/10/10"};
    REQUIRE(d.date_type() == date_type_e::INVALID);
    REQUIRE(d.value() == "2020/10/10");
    REQUIRE(d.ymd().ok() == false);
    REQUIRE(d.ymd().year() == chrono_year{0});
    REQUIRE(d.ymd().month() == chrono_month{0});
    REQUIRE(d.ymd().day() == chrono_day{0});
    REQUIRE(d.date().year == 0);
    REQUIRE(d.date().month == 0);
    REQUIRE(d.date().day == 0);
    REQUIRE(d.time().hour == 0);
    REQUIRE(d.time().minutes == 0);
    REQUIRE(d.time().seconds == 0);
    REQUIRE(d.time().millis == 0);
    REQUIRE(d.zone_info().hours == 0);
    REQUIRE(d.zone_info().minutes == 0);
}

TEST_CASE("Core Date: no iso8601 datetime", "[core_date]")
{
    iso_date d{"2020/10/10T10:10:10"};
    REQUIRE(d.date_type() == date_type_e::INVALID);
    REQUIRE(d.value() == "2020/10/10T10:10:10");
    REQUIRE(d.ymd().ok() == false);
    REQUIRE(d.ymd().year() == chrono_year{0});
    REQUIRE(d.ymd().month() == chrono_month{0});
    REQUIRE(d.ymd().day() == chrono_day{0});
    REQUIRE(d.date().year == 0);
    REQUIRE(d.date().month == 0);
    REQUIRE(d.date().day == 0);
    REQUIRE(d.time().hour == 0);
    REQUIRE(d.time().minutes == 0);
    REQUIRE(d.time().seconds == 0);
    REQUIRE(d.time().millis == 0);
    REQUIRE(d.zone_info().hours == 0);
    REQUIRE(d.zone_info().minutes == 0);
}

TEST_CASE("Core Date: no iso8601 datetime (with no T)", "[core_date]")
{
    iso_date d{"2020-10-10 10:10:10"};
    REQUIRE(d.date_type() == date_type_e::INVALID);
    REQUIRE(d.value() == "2020-10-10 10:10:10");
    REQUIRE(d.ymd().ok() == false);
    REQUIRE(d.ymd().year() == chrono_year{0});
    REQUIRE(d.ymd().month() == chrono_month{0});
    REQUIRE(d.ymd().day() == chrono_day{0});
    REQUIRE(d.date().year == 0);
    REQUIRE(d.date().month == 0);
    REQUIRE(d.date().day == 0);
    REQUIRE(d.time().hour == 0);
    REQUIRE(d.time().minutes == 0);
    REQUIRE(d.time().seconds == 0);
    REQUIRE(d.time().millis == 0);
    REQUIRE(d.zone_info().hours == 0);
    REQUIRE(d.zone_info().minutes == 0);
}

TEST_CASE("Core Date: no iso8601 datetime (with no numbers)", "[core_date]")
{
    iso_date d{"2020-10-O1T10:10:10"};
    REQUIRE(d.date_type() == date_type_e::INVALID);
    REQUIRE(d.value() == "2020-10-O1T10:10:10");
    REQUIRE(d.ymd().ok() == false);
    REQUIRE(d.ymd().year() == chrono_year{0});
    REQUIRE(d.ymd().month() == chrono_month{0});
    REQUIRE(d.ymd().day() == chrono_day{0});
    REQUIRE(d.date().year == 0);
    REQUIRE(d.date().month == 0);
    REQUIRE(d.date().day == 0);
    REQUIRE(d.time().hour == 0);
    REQUIRE(d.time().minutes == 0);
    REQUIRE(d.time().seconds == 0);
    REQUIRE(d.time().millis == 0);
    REQUIRE(d.zone_info().hours == 0);
    REQUIRE(d.zone_info().minutes == 0);
}

TEST_CASE("Core Date: not a valid year (two digits year)", "[core_date]")
{
    iso_date d{"20-10-01T10:10:10"};
    REQUIRE(d.date_type() == date_type_e::INVALID);
    REQUIRE(d.value() == "20-10-01T10:10:10");
    REQUIRE(d.ymd().ok() == false);
    REQUIRE(d.ymd().year() == chrono_year{0});
    REQUIRE(d.ymd().month() == chrono_month{0});
    REQUIRE(d.ymd().day() == chrono_day{0});
    REQUIRE(d.date().year == 0);
    REQUIRE(d.date().month == 0);
    REQUIRE(d.date().day == 0);
    REQUIRE(d.time().hour == 0);
    REQUIRE(d.time().minutes == 0);
    REQUIRE(d.time().seconds == 0);
    REQUIRE(d.time().millis == 0);
    REQUIRE(d.zone_info().hours == 0);
    REQUIRE(d.zone_info().minutes == 0);
}

TEST_CASE("Core Date: not a valid positive hour zone info", "[core_date]")
{
    iso_date d{"2020-10-10T09:10:36+25:00"};
    REQUIRE(d.date_type() == date_type_e::INVALID);
    REQUIRE(d.value() == "2020-10-10T09:10:36+25:00");
    REQUIRE(d.ymd().ok() == false);
    REQUIRE(d.ymd().year() == chrono_year{0});
    REQUIRE(d.ymd().month() == chrono_month{0});
    REQUIRE(d.ymd().day() == chrono_day{0});
    REQUIRE(d.date().year == 2020);
    REQUIRE(d.date().month == 10);
    REQUIRE(d.date().day == 10);
    REQUIRE(d.time().hour == 9);
    REQUIRE(d.time().minutes == 10);
    REQUIRE(d.time().seconds == 36);
    REQUIRE(d.time().millis == 0);
    REQUIRE(d.zone_info().hours == 25);
    REQUIRE(d.zone_info().minutes == 0);
}

TEST_CASE("Core Date: not a valid negative hour zone info", "[core_date]")
{
    iso_date d{"2020-10-10T09:10:36.987-25:00"};
    REQUIRE(d.date_type() == date_type_e::INVALID);
    REQUIRE(d.value() == "2020-10-10T09:10:36.987-25:00");
    REQUIRE(d.ymd().ok() == false);
    REQUIRE(d.ymd().year() == chrono_year{0});
    REQUIRE(d.ymd().month() == chrono_month{0});
    REQUIRE(d.ymd().day() == chrono_day{0});
    REQUIRE(d.date().year == 2020);
    REQUIRE(d.date().month == 10);
    REQUIRE(d.date().day == 10);
    REQUIRE(d.time().hour == 9);
    REQUIRE(d.time().minutes == 10);
    REQUIRE(d.time().seconds == 36);
    REQUIRE(d.time().millis == 987);
    REQUIRE(d.zone_info().hours == -25);
    REQUIRE(d.zone_info().minutes == 0);
}

TEST_CASE("Core Date: not a valid positive minutes zone info", "[core_date]")
{
    iso_date d{"2020-10-10T09:10:36.036+08:60"};
    REQUIRE(d.date_type() == date_type_e::INVALID);
    REQUIRE(d.value() == "2020-10-10T09:10:36.036+08:60");
    REQUIRE(d.ymd().ok() == false);
    REQUIRE(d.ymd().year() == chrono_year{0});
    REQUIRE(d.ymd().month() == chrono_month{0});
    REQUIRE(d.ymd().day() == chrono_day{0});
    REQUIRE(d.date().year == 2020);
    REQUIRE(d.date().month == 10);
    REQUIRE(d.date().day == 10);
    REQUIRE(d.time().hour == 9);
    REQUIRE(d.time().minutes == 10);
    REQUIRE(d.time().seconds == 36);
    REQUIRE(d.time().millis == 36);
    REQUIRE(d.zone_info().hours == 8);
    REQUIRE(d.zone_info().minutes == 60);
}

TEST_CASE("Core Date: not a valid negative minutes zone info", "[core_date]")
{
    iso_date d{"2020-10-10T09:10:36-10:60"};
    REQUIRE(d.date_type() == date_type_e::INVALID);
    REQUIRE(d.value() == "2020-10-10T09:10:36-10:60");
    REQUIRE(d.ymd().ok() == false);
    REQUIRE(d.ymd().year() == chrono_year{0});
    REQUIRE(d.ymd().month() == chrono_month{0});
    REQUIRE(d.ymd().day() == chrono_day{0});
    REQUIRE(d.date().year == 2020);
    REQUIRE(d.date().month == 10);
    REQUIRE(d.date().day == 10);
    REQUIRE(d.time().hour == 9);
    REQUIRE(d.time().minutes == 10);
    REQUIRE(d.time().seconds == 36);
    REQUIRE(d.time().millis == 0);
    REQUIRE(d.zone_info().hours == -10);
    REQUIRE(d.zone_info().minutes == 60);
}

TEST_CASE("Core Date: valid date", "[core_date]")
{
    iso_date d{"2020-10-10"};
    REQUIRE(d.date_type() == date_type_e::DATE);
    REQUIRE(d.value() == "2020-10-10");
    REQUIRE(d.ymd().ok() == true);
    REQUIRE(d.ymd().year() == chrono_year{2020});
    REQUIRE(d.ymd().month() == chrono_month{10});
    REQUIRE(d.ymd().day() == chrono_day{10});
    REQUIRE(d.date().year == 2020);
    REQUIRE(d.date().month == 10);
    REQUIRE(d.date().day == 10);
    REQUIRE(d.time().hour == 0);
    REQUIRE(d.time().minutes == 0);
    REQUIRE(d.time().seconds == 0);
    REQUIRE(d.time().millis == 0);
    REQUIRE(d.zone_info().hours == 0);
    REQUIRE(d.zone_info().minutes == 0);
}

TEST_CASE("Core Date: valid datetime", "[core_date]")
{
    iso_date d{"2020-10-10T09:10:36.009"};
    REQUIRE(d.date_type() == date_type_e::DATETIME);
    REQUIRE(d.value() == "2020-10-10T09:10:36.009");
    REQUIRE(d.ymd().ok() == true);
    REQUIRE(d.ymd().year() == chrono_year{2020});
    REQUIRE(d.ymd().month() == chrono_month{10});
    REQUIRE(d.ymd().day() == chrono_day{10});
    REQUIRE(d.date().year == 2020);
    REQUIRE(d.date().month == 10);
    REQUIRE(d.date().day == 10);
    REQUIRE(d.time().hour == 9);
    REQUIRE(d.time().minutes == 10);
    REQUIRE(d.time().seconds == 36);
    REQUIRE(d.time().millis == 9);
    REQUIRE(d.zone_info().hours == 0);
    REQUIRE(d.zone_info().minutes == 0);
}

TEST_CASE("Core Date: valid datetime with +2 zone info", "[core_date]")
{
    iso_date d{"2020-10-10T09:10:36+02:00"};
    REQUIRE(d.date_type() == date_type_e::DATETIME);
    REQUIRE(d.value() == "2020-10-10T09:10:36+02:00");
    REQUIRE(d.ymd().ok() == true);
    REQUIRE(d.ymd().year() == chrono_year{2020});
    REQUIRE(d.ymd().month() == chrono_month{10});
    REQUIRE(d.ymd().day() == chrono_day{10});
    REQUIRE(d.date().year == 2020);
    REQUIRE(d.date().month == 10);
    REQUIRE(d.date().day == 10);
    REQUIRE(d.time().hour == 9);
    REQUIRE(d.time().minutes == 10);
    REQUIRE(d.time().seconds == 36);
    REQUIRE(d.time().millis == 0);
    REQUIRE(d.zone_info().hours == 2);
    REQUIRE(d.zone_info().minutes == 0);
}

TEST_CASE("Core Date: valid datetime with -02:45 zone info", "[core_date]")
{
    iso_date d{"2020-10-10T09:10:36-02:45"};
    REQUIRE(d.date_type() == date_type_e::DATETIME);
    REQUIRE(d.value() == "2020-10-10T09:10:36-02:45");
    REQUIRE(d.ymd().ok() == true);
    REQUIRE(d.ymd().year() == chrono_year{2020});
    REQUIRE(d.ymd().month() == chrono_month{10});
    REQUIRE(d.ymd().day() == chrono_day{10});
    REQUIRE(d.date().year == 2020);
    REQUIRE(d.date().month == 10);
    REQUIRE(d.date().day == 10);
    REQUIRE(d.time().hour == 9);
    REQUIRE(d.time().minutes == 10);
    REQUIRE(d.time().seconds == 36);
    REQUIRE(d.time().millis == 0);
    REQUIRE(d.zone_info().hours == -2);
    REQUIRE(d.zone_info().minutes == 45);
}

TEST_CASE("Core Date: valid datetime with Z zone info", "[core_date]")
{
    iso_date d{"2020-10-10T09:10:36Z"};
    REQUIRE(d.date_type() == date_type_e::DATETIME);
    REQUIRE(d.value() == "2020-10-10T09:10:36Z");
    REQUIRE(d.ymd().ok() == true);
    REQUIRE(d.ymd().year() == chrono_year{2020});
    REQUIRE(d.ymd().month() == chrono_month{10});
    REQUIRE(d.ymd().day() == chrono_day{10});
    REQUIRE(d.date().year == 2020);
    REQUIRE(d.date().month == 10);
    REQUIRE(d.date().day == 10);
    REQUIRE(d.time().hour == 9);
    REQUIRE(d.time().minutes == 10);
    REQUIRE(d.time().seconds == 36);
    REQUIRE(d.time().millis == 0);
    REQUIRE(d.zone_info().hours == 0);
    REQUIRE(d.zone_info().minutes == 0);
}

TEST_CASE("Core Date: valid datetime with Z zone info and millis", "[core_date]")
{
    iso_date d{"2020-10-10T09:10:36.054Z"};
    REQUIRE(d.date_type() == date_type_e::DATETIME);
    REQUIRE(d.value() == "2020-10-10T09:10:36.054Z");
    REQUIRE(d.ymd().ok() == true);
    REQUIRE(d.ymd().year() == chrono_year{2020});
    REQUIRE(d.ymd().month() == chrono_month{10});
    REQUIRE(d.ymd().day() == chrono_day{10});
    REQUIRE(d.date().year == 2020);
    REQUIRE(d.date().month == 10);
    REQUIRE(d.date().day == 10);
    REQUIRE(d.time().hour == 9);
    REQUIRE(d.time().minutes == 10);
    REQUIRE(d.time().seconds == 36);
    REQUIRE(d.time().millis == 54);
    REQUIRE(d.zone_info().hours == 0);
    REQUIRE(d.zone_info().minutes == 0);
}

TEST_CASE("Core Date: invalid millis in datetime without zone info", "[core_date]")
{
    iso_date d{"2020-10-10T09:10:36.45"};
    REQUIRE(d.date_type() == date_type_e::INVALID);
    REQUIRE(d.value() == "2020-10-10T09:10:36.45");
    REQUIRE(d.ymd().ok() == false);
    REQUIRE(d.ymd().year() == chrono_year{0});
    REQUIRE(d.ymd().month() == chrono_month{0});
    REQUIRE(d.ymd().day() == chrono_day{0});
    REQUIRE(d.date().year == 0);
    REQUIRE(d.date().month == 0);
    REQUIRE(d.date().day == 0);
    REQUIRE(d.time().hour == 0);
    REQUIRE(d.time().minutes == 0);
    REQUIRE(d.time().seconds == 0);
    REQUIRE(d.time().millis == 0);
    REQUIRE(d.zone_info().hours == 0);
    REQUIRE(d.zone_info().minutes == 0);
}

TEST_CASE("Core Date: invalid millis in datetime with Z", "[core_date]")
{
    iso_date d{"2020-10-10T09:10:36.4545Z"};
    REQUIRE(d.date_type() == date_type_e::INVALID);
    REQUIRE(d.value() == "2020-10-10T09:10:36.4545Z");
    REQUIRE(d.ymd().ok() == false);
    REQUIRE(d.ymd().year() == chrono_year{0});
    REQUIRE(d.ymd().month() == chrono_month{0});
    REQUIRE(d.ymd().day() == chrono_day{0});
    REQUIRE(d.date().year == 0);
    REQUIRE(d.date().month == 0);
    REQUIRE(d.date().day == 0);
    REQUIRE(d.time().hour == 0);
    REQUIRE(d.time().minutes == 0);
    REQUIRE(d.time().seconds == 0);
    REQUIRE(d.time().millis == 0);
    REQUIRE(d.zone_info().hours == 0);
    REQUIRE(d.zone_info().minutes == 0);
}

TEST_CASE("Core Date: invalid millis in datetime with zone info", "[core_date]")
{
    iso_date d{"2020-10-10T09:10:36.5+02:30"};
    REQUIRE(d.date_type() == date_type_e::INVALID);
    REQUIRE(d.value() == "2020-10-10T09:10:36.5+02:30");
    REQUIRE(d.ymd().ok() == false);
    REQUIRE(d.ymd().year() == chrono_year{0});
    REQUIRE(d.ymd().month() == chrono_month{0});
    REQUIRE(d.ymd().day() == chrono_day{0});
    REQUIRE(d.date().year == 0);
    REQUIRE(d.date().month == 0);
    REQUIRE(d.date().day == 0);
    REQUIRE(d.time().hour == 0);
    REQUIRE(d.time().minutes == 0);
    REQUIRE(d.time().seconds == 0);
    REQUIRE(d.time().millis == 0);
    REQUIRE(d.zone_info().hours == 0);
    REQUIRE(d.zone_info().minutes == 0);
}

TEST_CASE("Core Date: decrement a simple date", "[core_date]")
{
    iso_date d{"2020-10-10"};
    REQUIRE(d.date_type() == date_type_e::DATE);
    REQUIRE(d.value() == "2020-10-10");
    REQUIRE(d.ymd().ok() == true);
    REQUIRE(d.ymd().year() == chrono_year{2020});
    REQUIRE(d.ymd().month() == chrono_month{10});
    REQUIRE(d.ymd().day() == chrono_day{10});
    REQUIRE(d.date().year == 2020);
    REQUIRE(d.date().month == 10);
    REQUIRE(d.date().day == 10);
    REQUIRE(d.time().hour == 0);
    REQUIRE(d.time().minutes == 0);
    REQUIRE(d.time().seconds == 0);
    REQUIRE(d.time().millis == 0);
    REQUIRE(d.zone_info().hours == 0);
    REQUIRE(d.zone_info().minutes == 0);

    --d;
    REQUIRE(d.date_type() == date_type_e::DATE);
    REQUIRE(d.value() == "2020-10-09");
    REQUIRE(d.ymd().ok() == true);
    REQUIRE(d.ymd().year() == chrono_year{2020});
    REQUIRE(d.ymd().month() == chrono_month{10});
    REQUIRE(d.ymd().day() == chrono_day{9});
    REQUIRE(d.date().year == 2020);
    REQUIRE(d.date().month == 10);
    REQUIRE(d.date().day == 9);
    REQUIRE(d.time().hour == 0);
    REQUIRE(d.time().minutes == 0);
    REQUIRE(d.time().seconds == 0);
    REQUIRE(d.time().millis == 0);
    REQUIRE(d.zone_info().hours == 0);
    REQUIRE(d.zone_info().minutes == 0);
}

TEST_CASE("Core Date: decrement with month changing", "[core_date]")
{
    iso_date d{"2020-10-01"};
    REQUIRE(d.date_type() == date_type_e::DATE);
    REQUIRE(d.value() == "2020-10-01");
    REQUIRE(d.ymd().ok() == true);
    REQUIRE(d.ymd().year() == chrono_year{2020});
    REQUIRE(d.ymd().month() == chrono_month{10});
    REQUIRE(d.ymd().day() == chrono_day{1});
    REQUIRE(d.date().year == 2020);
    REQUIRE(d.date().month == 10);
    REQUIRE(d.date().day == 1);
    REQUIRE(d.time().hour == 0);
    REQUIRE(d.time().minutes == 0);
    REQUIRE(d.time().seconds == 0);
    REQUIRE(d.time().millis == 0);
    REQUIRE(d.zone_info().hours == 0);
    REQUIRE(d.zone_info().minutes == 0);

    --d;
    REQUIRE(d.date_type() == date_type_e::DATE);
    REQUIRE(d.value() == "2020-09-30");
    REQUIRE(d.ymd().ok() == true);
    REQUIRE(d.ymd().year() == chrono_year{2020});
    REQUIRE(d.ymd().month() == chrono_month{9});
    REQUIRE(d.ymd().day() == chrono_day{30});
    REQUIRE(d.date().year == 2020);
    REQUIRE(d.date().month == 9);
    REQUIRE(d.date().day == 30);
    REQUIRE(d.time().hour == 0);
    REQUIRE(d.time().minutes == 0);
    REQUIRE(d.time().seconds == 0);
    REQUIRE(d.time().millis == 0);
    REQUIRE(d.zone_info().hours == 0);
    REQUIRE(d.zone_info().minutes == 0);
}

TEST_CASE("Core Date: decrement with month and year changing", "[core_date]")
{
    iso_date d{"2020-01-01T10:10:10+05:05"};
    REQUIRE(d.date_type() == date_type_e::DATETIME);
    REQUIRE(d.value() == "2020-01-01T10:10:10+05:05");
    REQUIRE(d.ymd().ok() == true);
    REQUIRE(d.ymd().year() == chrono_year{2020});
    REQUIRE(d.ymd().month() == chrono_month{1});
    REQUIRE(d.ymd().day() == chrono_day{1});
    REQUIRE(d.date().year == 2020);
    REQUIRE(d.date().month == 1);
    REQUIRE(d.date().day == 1);
    REQUIRE(d.time().hour == 10);
    REQUIRE(d.time().minutes == 10);
    REQUIRE(d.time().seconds == 10);
    REQUIRE(d.time().millis == 0);
    REQUIRE(d.zone_info().hours == 5);
    REQUIRE(d.zone_info().minutes == 5);

    --d;
    REQUIRE(d.date_type() == date_type_e::DATETIME);
    REQUIRE(d.value() == "2019-12-31T10:10:10+05:05");
    REQUIRE(d.ymd().ok() == true);
    REQUIRE(d.ymd().year() == chrono_year{2019});
    REQUIRE(d.ymd().month() == chrono_month{12});
    REQUIRE(d.ymd().day() == chrono_day{31});
    REQUIRE(d.date().year == 2019);
    REQUIRE(d.date().month == 12);
    REQUIRE(d.date().day == 31);
    REQUIRE(d.time().hour == 10);
    REQUIRE(d.time().minutes == 10);
    REQUIRE(d.time().seconds == 10);
    REQUIRE(d.time().millis == 0);
    REQUIRE(d.zone_info().hours == 5);
    REQUIRE(d.zone_info().minutes == 5);
}

TEST_CASE("Core Date: decrement to leap feb month", "[core_date]")
{
    iso_date d{"2020-03-01T10:10:10-05:05"};
    REQUIRE(d.date_type() == date_type_e::DATETIME);
    REQUIRE(d.value() == "2020-03-01T10:10:10-05:05");
    REQUIRE(d.ymd().ok() == true);
    REQUIRE(d.ymd().year() == chrono_year{2020});
    REQUIRE(d.ymd().month() == chrono_month{3});
    REQUIRE(d.ymd().day() == chrono_day{1});
    REQUIRE(d.date().year == 2020);
    REQUIRE(d.date().month == 3);
    REQUIRE(d.date().day == 1);
    REQUIRE(d.time().hour == 10);
    REQUIRE(d.time().minutes == 10);
    REQUIRE(d.time().seconds == 10);
    REQUIRE(d.time().millis == 0);
    REQUIRE(d.zone_info().hours == -5);
    REQUIRE(d.zone_info().minutes == 5);

    --d;
    REQUIRE(d.date_type() == date_type_e::DATETIME);
    REQUIRE(d.value() == "2020-02-29T10:10:10-05:05");
    REQUIRE(d.ymd().ok() == true);
    REQUIRE(d.ymd().year() == chrono_year{2020});
    REQUIRE(d.ymd().month() == chrono_month{2});
    REQUIRE(d.ymd().day() == chrono_day{29});
    REQUIRE(d.date().year == 2020);
    REQUIRE(d.date().month == 2);
    REQUIRE(d.date().day == 29);
    REQUIRE(d.time().hour == 10);
    REQUIRE(d.time().minutes == 10);
    REQUIRE(d.time().seconds == 10);
    REQUIRE(d.time().millis == 0);
    REQUIRE(d.zone_info().hours == -5);
    REQUIRE(d.zone_info().minutes == 5);
}

TEST_CASE("Core Date: decrement to not leap feb month", "[core_date]")
{
    iso_date d{"2018-03-01T10:10:10-05:05"};
    REQUIRE(d.date_type() == date_type_e::DATETIME);
    REQUIRE(d.value() == "2018-03-01T10:10:10-05:05");
    REQUIRE(d.ymd().ok() == true);
    REQUIRE(d.ymd().year() == chrono_year{2018});
    REQUIRE(d.ymd().month() == chrono_month{3});
    REQUIRE(d.ymd().day() == chrono_day{1});
    REQUIRE(d.date().year == 2018);
    REQUIRE(d.date().month == 3);
    REQUIRE(d.date().day == 1);
    REQUIRE(d.time().hour == 10);
    REQUIRE(d.time().minutes == 10);
    REQUIRE(d.time().seconds == 10);
    REQUIRE(d.time().millis == 0);
    REQUIRE(d.zone_info().hours == -5);
    REQUIRE(d.zone_info().minutes == 5);

    --d;
    REQUIRE(d.date_type() == date_type_e::DATETIME);
    REQUIRE(d.value() == "2018-02-28T10:10:10-05:05");
    REQUIRE(d.ymd().ok() == true);
    REQUIRE(d.ymd().year() == chrono_year{2018});
    REQUIRE(d.ymd().month() == chrono_month{2});
    REQUIRE(d.ymd().day() == chrono_day{28});
    REQUIRE(d.date().year == 2018);
    REQUIRE(d.date().month == 2);
    REQUIRE(d.date().day == 28);
    REQUIRE(d.time().hour == 10);
    REQUIRE(d.time().minutes == 10);
    REQUIRE(d.time().seconds == 10);
    REQUIRE(d.time().millis == 0);
    REQUIRE(d.zone_info().hours == -5);
    REQUIRE(d.zone_info().minutes == 5);
}

TEST_CASE("Core Date: less than", "[core_date]")
{
    iso_date d1{"2018-03-01"};
    iso_date d2{"2018-03-01"};
    REQUIRE(!(d1 < d2));

    d1 = iso_date{"2020-03-01"};
    d2 = iso_date{"2020-02-29"};
    REQUIRE(!(d1 < d2));

    d1 = iso_date{"2020-02-29"};
    d2 = iso_date{"2020-03-01"};
    REQUIRE(d1 < d2);

    d1 = iso_date{"2020-02-29T10:10:10Z"};
    d2 = iso_date{"2020-02-29T10:10:11Z"};
    REQUIRE(d1 < d2);

    d1 = iso_date{"2020-02-29T00:00:00Z"};
    d2 = iso_date{"2020-02-29"};
    REQUIRE(!(d1 < d2));
    REQUIRE(!(d2 < d1));

    d1 = iso_date{"2020-02-29T00:00:00+00:00"};
    d2 = iso_date{"2020-02-29"};
    REQUIRE(!(d1 < d2));
    REQUIRE(!(d2 < d1));

    d1 = iso_date{"2020-02-29T02:00:00+02:00"};
    d2 = iso_date{"2020-02-29T00:00:00Z"};
    REQUIRE(!(d1 < d2));
    REQUIRE(!(d2 < d1));

    d1 = iso_date{"2020-02-28T22:00:00-02:00"};
    d2 = iso_date{"2020-02-29T00:00:00"};
    REQUIRE(!(d1 < d2));
    REQUIRE(!(d2 < d1));

    d1 = iso_date{"2020-02-28T21:30:00-02:30"};
    d2 = iso_date{"2020-02-29T00:00:00"};
    REQUIRE(!(d1 < d2));
    REQUIRE(!(d2 < d1));

    d1 = iso_date{"2020-02-29T21:10:00+11:00"};
    d2 = iso_date{"2020-02-29T10:10:00Z"};
    REQUIRE(!(d1 < d2));
    REQUIRE(!(d2 < d1));

    d1 = iso_date{"2020-02-10T22:10:00-01:50"};
    d2 = iso_date{"2020-02-11T10:10:00+10:10"};
    REQUIRE(!(d1 < d2));
    REQUIRE(!(d2 < d1));

    d1 = iso_date{"2020-02-10T22:10:30-01:50"};
    d2 = iso_date{"2020-02-11T23:59:30+23:59"};
    REQUIRE(!(d1 < d2));
    REQUIRE(!(d2 < d1));

    d1 = iso_date{"2020-02-10T22:10:30-01:51"};
    d2 = iso_date{"2020-02-11T23:59:30+23:59"};
    REQUIRE(!(d1 < d2));
    REQUIRE(d2 < d1);
}

TEST_CASE("Core Date: increment a simple date", "[core_date]")
{
    iso_date d{"2020-10-10"};
    REQUIRE(d.date_type() == date_type_e::DATE);
    REQUIRE(d.value() == "2020-10-10");
    REQUIRE(d.ymd().ok() == true);
    REQUIRE(d.ymd().year() == chrono_year{2020});
    REQUIRE(d.ymd().month() == chrono_month{10});
    REQUIRE(d.ymd().day() == chrono_day{10});
    REQUIRE(d.date().year == 2020);
    REQUIRE(d.date().month == 10);
    REQUIRE(d.date().day == 10);
    REQUIRE(d.time().hour == 0);
    REQUIRE(d.time().minutes == 0);
    REQUIRE(d.time().seconds == 0);
    REQUIRE(d.time().millis == 0);
    REQUIRE(d.zone_info().hours == 0);
    REQUIRE(d.zone_info().minutes == 0);

    ++d;
    REQUIRE(d.date_type() == date_type_e::DATE);
    REQUIRE(d.value() == "2020-10-11");
    REQUIRE(d.ymd().ok() == true);
    REQUIRE(d.ymd().year() == chrono_year{2020});
    REQUIRE(d.ymd().month() == chrono_month{10});
    REQUIRE(d.ymd().day() == chrono_day{11});
    REQUIRE(d.date().year == 2020);
    REQUIRE(d.date().month == 10);
    REQUIRE(d.date().day == 11);
    REQUIRE(d.time().hour == 0);
    REQUIRE(d.time().minutes == 0);
    REQUIRE(d.time().seconds == 0);
    REQUIRE(d.time().millis == 0);
    REQUIRE(d.zone_info().hours == 0);
    REQUIRE(d.zone_info().minutes == 0);
}

TEST_CASE("Core Date: increment with month changing", "[core_date]")
{
    iso_date d{"2020-10-31"};
    REQUIRE(d.date_type() == date_type_e::DATE);
    REQUIRE(d.value() == "2020-10-31");
    REQUIRE(d.ymd().ok() == true);
    REQUIRE(d.ymd().year() == chrono_year{2020});
    REQUIRE(d.ymd().month() == chrono_month{10});
    REQUIRE(d.ymd().day() == chrono_day{31});
    REQUIRE(d.date().year == 2020);
    REQUIRE(d.date().month == 10);
    REQUIRE(d.date().day == 31);
    REQUIRE(d.time().hour == 0);
    REQUIRE(d.time().minutes == 0);
    REQUIRE(d.time().seconds == 0);
    REQUIRE(d.time().millis == 0);
    REQUIRE(d.zone_info().hours == 0);
    REQUIRE(d.zone_info().minutes == 0);

    ++d;
    REQUIRE(d.date_type() == date_type_e::DATE);
    REQUIRE(d.value() == "2020-11-01");
    REQUIRE(d.ymd().ok() == true);
    REQUIRE(d.ymd().year() == chrono_year{2020});
    REQUIRE(d.ymd().month() == chrono_month{11});
    REQUIRE(d.ymd().day() == chrono_day{1});
    REQUIRE(d.date().year == 2020);
    REQUIRE(d.date().month == 11);
    REQUIRE(d.date().day == 1);
    REQUIRE(d.time().hour == 0);
    REQUIRE(d.time().minutes == 0);
    REQUIRE(d.time().seconds == 0);
    REQUIRE(d.time().millis == 0);
    REQUIRE(d.zone_info().hours == 0);
    REQUIRE(d.zone_info().minutes == 0);
}

TEST_CASE("Core Date: increment with month and year changing", "[core_date]")
{
    iso_date d{"2020-12-31T10:10:10+05:05"};
    REQUIRE(d.date_type() == date_type_e::DATETIME);
    REQUIRE(d.value() == "2020-12-31T10:10:10+05:05");
    REQUIRE(d.ymd().ok() == true);
    REQUIRE(d.ymd().year() == chrono_year{2020});
    REQUIRE(d.ymd().month() == chrono_month{12});
    REQUIRE(d.ymd().day() == chrono_day{31});
    REQUIRE(d.date().year == 2020);
    REQUIRE(d.date().month == 12);
    REQUIRE(d.date().day == 31);
    REQUIRE(d.time().hour == 10);
    REQUIRE(d.time().minutes == 10);
    REQUIRE(d.time().seconds == 10);
    REQUIRE(d.time().millis == 0);
    REQUIRE(d.zone_info().hours == 5);
    REQUIRE(d.zone_info().minutes == 5);

    ++d;
    REQUIRE(d.date_type() == date_type_e::DATETIME);
    REQUIRE(d.value() == "2021-01-01T10:10:10+05:05");
    REQUIRE(d.ymd().ok() == true);
    REQUIRE(d.ymd().year() == chrono_year{2021});
    REQUIRE(d.ymd().month() == chrono_month{1});
    REQUIRE(d.ymd().day() == chrono_day{1});
    REQUIRE(d.date().year == 2021);
    REQUIRE(d.date().month == 1);
    REQUIRE(d.date().day == 1);
    REQUIRE(d.time().hour == 10);
    REQUIRE(d.time().minutes == 10);
    REQUIRE(d.time().seconds == 10);
    REQUIRE(d.time().millis == 0);
    REQUIRE(d.zone_info().hours == 5);
    REQUIRE(d.zone_info().minutes == 5);
}

TEST_CASE("Core Date: increment to leap feb month", "[core_date]")
{
    iso_date d{"2020-02-28T10:10:10-05:05"};
    REQUIRE(d.date_type() == date_type_e::DATETIME);
    REQUIRE(d.value() == "2020-02-28T10:10:10-05:05");
    REQUIRE(d.ymd().ok() == true);
    REQUIRE(d.ymd().year() == chrono_year{2020});
    REQUIRE(d.ymd().month() == chrono_month{2});
    REQUIRE(d.ymd().day() == chrono_day{28});
    REQUIRE(d.date().year == 2020);
    REQUIRE(d.date().month == 2);
    REQUIRE(d.date().day == 28);
    REQUIRE(d.time().hour == 10);
    REQUIRE(d.time().minutes == 10);
    REQUIRE(d.time().seconds == 10);
    REQUIRE(d.time().millis == 0);
    REQUIRE(d.zone_info().hours == -5);
    REQUIRE(d.zone_info().minutes == 5);

    ++d;
    REQUIRE(d.date_type() == date_type_e::DATETIME);
    REQUIRE(d.value() == "2020-02-29T10:10:10-05:05");
    REQUIRE(d.ymd().ok() == true);
    REQUIRE(d.ymd().year() == chrono_year{2020});
    REQUIRE(d.ymd().month() == chrono_month{2});
    REQUIRE(d.ymd().day() == chrono_day{29});
    REQUIRE(d.date().year == 2020);
    REQUIRE(d.date().month == 2);
    REQUIRE(d.date().day == 29);
    REQUIRE(d.time().hour == 10);
    REQUIRE(d.time().minutes == 10);
    REQUIRE(d.time().seconds == 10);
    REQUIRE(d.time().millis == 0);
    REQUIRE(d.zone_info().hours == -5);
    REQUIRE(d.zone_info().minutes == 5);
}

TEST_CASE("Core Date: increment to not leap feb month", "[core_date]")
{
    iso_date d{"2018-02-28T10:10:10-05:05"};
    REQUIRE(d.date_type() == date_type_e::DATETIME);
    REQUIRE(d.value() == "2018-02-28T10:10:10-05:05");
    REQUIRE(d.ymd().ok() == true);
    REQUIRE(d.ymd().year() == chrono_year{2018});
    REQUIRE(d.ymd().month() == chrono_month{2});
    REQUIRE(d.ymd().day() == chrono_day{28});
    REQUIRE(d.date().year == 2018);
    REQUIRE(d.date().month == 2);
    REQUIRE(d.date().day == 28);
    REQUIRE(d.time().hour == 10);
    REQUIRE(d.time().minutes == 10);
    REQUIRE(d.time().seconds == 10);
    REQUIRE(d.time().millis == 0);
    REQUIRE(d.zone_info().hours == -5);
    REQUIRE(d.zone_info().minutes == 5);

    ++d;
    REQUIRE(d.date_type() == date_type_e::DATETIME);
    REQUIRE(d.value() == "2018-03-01T10:10:10-05:05");
    REQUIRE(d.ymd().ok() == true);
    REQUIRE(d.ymd().year() == chrono_year{2018});
    REQUIRE(d.ymd().month() == chrono_month{3});
    REQUIRE(d.ymd().day() == chrono_day{1});
    REQUIRE(d.date().year == 2018);
    REQUIRE(d.date().month == 3);
    REQUIRE(d.date().day == 1);
    REQUIRE(d.time().hour == 10);
    REQUIRE(d.time().minutes == 10);
    REQUIRE(d.time().seconds == 10);
    REQUIRE(d.time().millis == 0);
    REQUIRE(d.zone_info().hours == -5);
    REQUIRE(d.zone_info().minutes == 5);
}

TEST_CASE("Core Date: from not valid date to utc", "[core_date]")
{
    iso_date d{"is nos a valid date"};
    REQUIRE(d.utc().value() == "0000-00-00T00:00:00Z");
}

TEST_CASE("Core Date: from date to utc", "[core_date]")
{
    iso_date d{"2024-03-31"};
    REQUIRE(d.utc().value() == "2024-03-31T00:00:00Z");
}

TEST_CASE("Core Date: from datetime without offset to utc", "[core_date]")
{
    iso_date d{"2024-03-31T12:13:14"};
    REQUIRE(d.utc().value() == "2024-03-31T12:13:14Z");
}

TEST_CASE("Core Date: from datetime with offset -05:00 to utc", "[core_date]")
{
    iso_date d{"2024-03-31T15:30:00-05:00"};
    REQUIRE(d.utc().value() == "2024-03-31T20:30:00Z");
}

TEST_CASE("Core Date: from datetime with offset +02:30 to utc", "[core_date]")
{
    iso_date d{"2023-07-15T10:45:00+02:30"};
    REQUIRE(d.utc().value() == "2023-07-15T08:15:00Z");
}

TEST_CASE("Core Date: from datetime in a utc format to utc", "[core_date]")
{
    iso_date d{"2025-12-01T23:59:59Z"};
    REQUIRE(d.utc().value() == "2025-12-01T23:59:59Z");
}

TEST_CASE("Core Date: from datetime with offset -06:00 to utc with day change", "[core_date]")
{
    iso_date d{"2023-03-31T23:30:00-06:00"};
    REQUIRE(d.utc().value() == "2023-04-01T05:30:00Z");
}

TEST_CASE("Core Date: from datetime with offset -04:00 to utc with day change", "[core_date]")
{
    iso_date d{"2025-11-30T23:45:00-04:00"};
    REQUIRE(d.utc().value() == "2025-12-01T03:45:00Z");
}

TEST_CASE("Core Date: from datetime with offset -03:00 to utc with day change", "[core_date]")
{
    iso_date d{"2024-06-30T23:59:59-01:00"};
    REQUIRE(d.utc().value() == "2024-07-01T00:59:59Z");
}

TEST_CASE("Core Date: from datetime in a utc with 0 millis format to utc", "[core_date]")
{
    iso_date d{"2025-12-01T23:59:59.000Z"};
    REQUIRE(d.utc().value() == "2025-12-01T23:59:59Z");
}

TEST_CASE("Core Date: from datetime in a utc with millis format to utc", "[core_date]")
{
    iso_date d{"2025-12-01T23:59:59.001Z"};
    REQUIRE(d.utc().value() == "2025-12-01T23:59:59.001Z");
}
