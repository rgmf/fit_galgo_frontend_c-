#include "api.h"

namespace fitgalgo
{

DateIdx::DateIdx(const std::string& value)
{
    if (value.size() < 10)
	return;

    if (value.size() >= 19)
	this->set_datetime_if_valid_value(value.substr(0, 19), "%Y-%m-%dT%H:%M:%S");

    if (this->datetime.empty())
	this->set_datetime_if_valid_value(value.substr(0, 10) + "T00:00:00", "%Y-%m-%d");
}

void DateIdx::set_datetime_if_valid_value(const std::string& value, const std::string& format)
{
    std::tm tm_struct{};
    std::istringstream iss(value);
    iss >> std::get_time(&tm_struct, format.c_str());
    if (!iss.fail())
	this->datetime = value;
}

std::chrono::year_month_day DateIdx::ymd() const
{
    std::istringstream ss(this->datetime);

    int year, month, day;
    char dash1, dash2;
    ss >> year >> dash1 >> month >> dash2 >> day;

    return std::chrono::year(year) / std::chrono::month(month) / std::chrono::day(day);
}

inline void DateIdx::decrement_date()
{
    auto ymd = this->ymd();
    auto yesterday = std::chrono::year_month_day(std::chrono::sys_days(ymd) - std::chrono::days(1));
    std::ostringstream oss;
    oss << std::setw(4) << std::setfill('0') << static_cast<int>(yesterday.year())
        << '-' << std::setw(2) << std::setfill('0') << static_cast<unsigned>(yesterday.month())
	<< '-' << std::setw(2) << std::setfill('0') << static_cast<unsigned>(yesterday.day());
    this->datetime = oss.str();
}

const std::string &DateIdx::value() const
{
    return this->datetime;
}

std::string DateIdx::year() const
{
    return is_valid() ? datetime.substr(0, 4) : "";
}

std::string DateIdx::month() const
{
    return is_valid() ? datetime.substr(5, 2) : "";
}

std::string DateIdx::day() const
{
    return is_valid() ? datetime.substr(8, 2) : "";
}

bool DateIdx::is_valid() const
{
    return !this->datetime.empty();
}

DateIdx& DateIdx::operator--()
{
    this->decrement_date();
    return *this;
}

DateIdx& DateIdx::operator--(int)
{
    this->decrement_date();
    return *this;
}

bool operator<(const DateIdx& l, const DateIdx& r)
{
    return l.datetime < r.datetime;
}

bool LoginData::load(const rapidjson::Document& document)
{
    const auto at_member = document.FindMember("access_token");
    if (at_member == document.MemberEnd() || !at_member->value.IsString())
    {
	return false;
    }
    this->access_token = at_member->value.GetString();

    const auto v = document.FindMember("token_type");
    if (v != document.MemberEnd() && v->value.IsString())
    {
	this->token_type = v->value.GetString();
    }

    return true;
}

bool UploadedFileData::load(const rapidjson::Document& document)
{
    const auto data = document.FindMember("data");
    if (data == document.MemberEnd() || !data->value.IsArray())
    {
	return false;
    }

    const auto itr = data->value.GetArray().Begin();
    const auto id = itr->FindMember("id");
    const auto file_path = itr->FindMember("file_path");
    const auto accepted = itr->FindMember("accepted");
    const auto zip_file_path = itr->FindMember("zip_file_path");
    const auto errors = itr->FindMember("errors");

    if (id != itr->MemberEnd() && id->value.IsString())
    {
	this->id = id->value.GetString();
    }

    if (file_path != itr->MemberEnd() && file_path->value.IsString())
    {
	this->file_path = file_path->value.GetString();
    }

    if (accepted != itr->MemberEnd() && accepted->value.IsBool())
    {
	this->accepted = accepted->value.GetBool();
    }

    if (zip_file_path != itr->MemberEnd() && zip_file_path->value.IsString())
    {
	this->zip_file_path = zip_file_path->value.GetString();
    }

    if (errors != itr->MemberEnd() && errors->value.IsArray())
    {
	for (const auto& v : errors->value.GetArray())
	{
	    if (v.IsString())
	    {
		this->errors.emplace_back(v.GetString());
	    }
	}
    }

    return true;
}

Steps& Steps::operator+=(const Steps& rhs)
{
    if (this->datetime_utc.empty())
	this->datetime_utc = rhs.datetime_utc;
    if (this->datetime_local.empty())
	this->datetime_local = rhs.datetime_local;
    this->steps += rhs.steps;
    this->distance += rhs.distance;
    this->calories += rhs.calories;
    return *this;
}

Steps operator+(Steps lhs, const Steps& rhs)
{
    lhs += rhs;
    return lhs;
}

bool StepsData::load(const rapidjson::Document& document)
{
    const auto data = document.FindMember("data");
    if (data == document.MemberEnd() || !data->value.IsArray())
    {
	return false;
    }

    for (const auto& v : data->value.GetArray())
    {
	DateIdx idx{v["datetime_local"].GetString()};
	if (v.IsObject() && v.HasMember("datetime_local") && idx.is_valid())
	{
	    auto itr_dt_utc = v.FindMember("datetime_utc");
	    auto itr_dt_local = v.FindMember("datetime_local");
	    auto itr_steps = v.FindMember("total_steps");
	    auto itr_distance = v.FindMember("total_distance");
	    auto itr_calories = v.FindMember("total_calories");

	    struct Steps item{};

	    item.datetime_local = itr_dt_local->value.GetString();

	    if (itr_dt_utc != v.MemberEnd() && itr_dt_utc->value.IsString())
		item.datetime_utc = itr_dt_utc->value.GetString();

	    if (itr_steps != v.MemberEnd() && itr_steps->value.IsInt())
		item.steps = itr_steps->value.GetInt();

            if (itr_distance != v.MemberEnd() && itr_distance->value.IsFloat())
		item.distance = itr_distance->value.GetFloat();

	    if (itr_calories != v.MemberEnd() && itr_calories->value.IsInt())
		item.calories = itr_calories->value.GetInt();

	    this->steps[idx] = item;
	}
	else
	{
	    this->errors.emplace_back(
		"JSON error: a datetime_local item is not an object, not exists or is not valid");
	}
    }

    return true;
}

SleepAssessment& SleepAssessment::operator+=(const SleepAssessment& rhs)
{
    this->combined_awake_score += rhs.combined_awake_score;
    this->awake_time_score += rhs.awake_time_score;	
    this->awakenings_count_score += rhs.awakenings_count_score;	
    this->deep_sleep_score += rhs.deep_sleep_score;	
    this->sleep_duration_score += rhs.sleep_duration_score;	
    this->light_sleep_score += rhs.light_sleep_score;	
    this->overall_sleep_score += rhs.overall_sleep_score;	
    this->sleep_quality_score += rhs.sleep_quality_score;	
    this->sleep_recovery_score += rhs.sleep_recovery_score;	
    this->rem_sleep_score += rhs.rem_sleep_score;	
    this->sleep_restlessness_score += rhs.sleep_restlessness_score;
    this->awakenings_count += rhs.awakenings_count;
    this->interruptions_score += rhs.interruptions_score;	
    this->average_stress_during_sleep += rhs.average_stress_during_sleep;

    return *this;
}

SleepAssessment operator+(SleepAssessment lhs, const SleepAssessment& rhs)
{
    lhs += rhs;
    return lhs;
}

SleepAssessment& SleepAssessment::operator/(int n)
{
    this->combined_awake_score /= n;
    this->awake_time_score /= n;
    this->awakenings_count_score /= n;
    this->deep_sleep_score /= n;
    this->sleep_duration_score /= n;
    this->light_sleep_score /= n;
    this->overall_sleep_score /= n;
    this->sleep_quality_score /= n;
    this->sleep_recovery_score /= n;
    this->rem_sleep_score /= n;
    this->sleep_restlessness_score /= n;
    this->awakenings_count /= n;
    this->interruptions_score /= n;
    this->average_stress_during_sleep /= n;
    return *this;
}

bool Sleep::is_early_morning() const
{
    if (this->dates.size() != 2)
	return false;

    if (this->dates[0].size() < 13)
	return false;

    std::stringstream ss{this->dates[0].substr(11, 2)};
    unsigned short num;
    if (ss >> num && num <= 6)
	return true;
    else
	return false;
}

Sleep& Sleep::operator+=(const Sleep& rhs)
{
    if (this->zone_info.empty())
	this->zone_info = rhs.zone_info;
    this->assessment += rhs.assessment;
    this->levels.insert(this->levels.end(), rhs.levels.begin(), rhs.levels.end());
    this->dates.insert(this->dates.end(), rhs.dates.begin(), rhs.dates.end());
    return *this;
}

Sleep operator+(Sleep lhs, const Sleep& rhs)
{
    lhs += rhs;
    return lhs;
}

Sleep& Sleep::operator/(int n)
{
    this->assessment = this->assessment / n;
    return *this;
}

bool SleepData::load(const rapidjson::Document& document)
{
    const auto data = document.FindMember("data");
    if (data == document.MemberEnd() || !data->value.IsArray())
    {
	return false;
    }

    for (const auto& v : data->value.GetArray())
    {
	if (v.IsObject())
	{
	    auto itr_zone_info = v.FindMember("zone_info");
	    auto itr_assessment = v.FindMember("assessment");
	    auto itr_levels = v.FindMember("levels");
	    auto itr_dates = v.FindMember("dates");

	    Sleep sleep_item{};

	    if (itr_zone_info != v.MemberEnd() && itr_zone_info->value.IsString())
	    {
		sleep_item.zone_info = itr_zone_info->value.GetString();
	    }

	    if (itr_assessment != v.MemberEnd() && itr_assessment->value.IsObject())
	    {
		auto itr_cas = itr_assessment->value.FindMember("combined_awake_score");
		auto itr_ats = itr_assessment->value.FindMember("awake_time_score");
		auto itr_acs = itr_assessment->value.FindMember("awakenings_count_score");
		auto itr_dss = itr_assessment->value.FindMember("deep_sleep_score");
		auto itr_sds = itr_assessment->value.FindMember("sleep_duration_score");
		auto itr_lss = itr_assessment->value.FindMember("light_sleep_score");
		auto itr_oss = itr_assessment->value.FindMember("overall_sleep_score");
		auto itr_sqs = itr_assessment->value.FindMember("sleep_quality_score");
		auto itr_srs = itr_assessment->value.FindMember("sleep_recovery_score");
		auto itr_rss = itr_assessment->value.FindMember("rem_sleep_score");
		auto itr_sres = itr_assessment->value.FindMember("sleep_restlessness_score");
		auto itr_ac = itr_assessment->value.FindMember("awakenings_count");
		auto itr_is = itr_assessment->value.FindMember("interruptions_score");
		auto itr_asds = itr_assessment->value.FindMember("average_stress_during_sleep");

		struct SleepAssessment assessment{};

		if (itr_cas != itr_assessment->value.MemberEnd() && itr_cas->value.IsInt())
		    assessment.combined_awake_score = itr_cas->value.GetInt();

		if (itr_ats != itr_assessment->value.MemberEnd() && itr_ats->value.IsInt())
		    assessment.awake_time_score = itr_ats->value.GetInt();

		if (itr_acs != itr_assessment->value.MemberEnd() && itr_acs->value.IsInt())
		    assessment.awakenings_count_score = itr_acs->value.GetInt();

		if (itr_dss != itr_assessment->value.MemberEnd() && itr_dss->value.IsInt())
		    assessment.deep_sleep_score = itr_dss->value.GetInt();

		if (itr_sds != itr_assessment->value.MemberEnd() && itr_sds->value.IsInt())
		    assessment.sleep_duration_score = itr_sds->value.GetInt();

		if (itr_lss != itr_assessment->value.MemberEnd() && itr_lss->value.IsInt())
		    assessment.light_sleep_score = itr_lss->value.GetInt();

		if (itr_oss != itr_assessment->value.MemberEnd() && itr_oss->value.IsInt())
		    assessment.overall_sleep_score = itr_oss->value.GetInt();

		if (itr_sqs != itr_assessment->value.MemberEnd() && itr_sqs->value.IsInt())
		    assessment.sleep_quality_score = itr_sqs->value.GetInt();
		
		if (itr_srs != itr_assessment->value.MemberEnd() && itr_srs->value.IsInt())
		    assessment.sleep_recovery_score = itr_srs->value.GetInt();

		if (itr_rss != itr_assessment->value.MemberEnd() && itr_rss->value.IsInt())
		    assessment.rem_sleep_score = itr_rss->value.GetInt();

		if (itr_sres != itr_assessment->value.MemberEnd() && itr_sres->value.IsInt())
		    assessment.sleep_restlessness_score = itr_sres->value.GetInt();

		if (itr_ac != itr_assessment->value.MemberEnd() && itr_ac->value.IsInt())
		    assessment.awakenings_count = itr_ac->value.GetInt();

		if (itr_is != itr_assessment->value.MemberEnd() && itr_is->value.IsInt())
		    assessment.interruptions_score = itr_is->value.GetInt();

		if (itr_asds != itr_assessment->value.MemberEnd() && itr_asds->value.IsFloat())
		    assessment.average_stress_during_sleep = itr_asds->value.GetFloat();

		sleep_item.assessment = assessment;
	    }

	    if (itr_levels != v.MemberEnd() && itr_levels->value.IsArray())
	    {
		std::vector<SleepLevel> levels{};
		for (const auto& l : itr_levels->value.GetArray())
		{
		    if (l.IsObject())
		    {
			auto itr_dt_utc = l.FindMember("datetime_utc");
			auto itr_level = l.FindMember("level");

			SleepLevel level{};

			if (itr_dt_utc != l.MemberEnd() && itr_dt_utc->value.IsString())
			    level.datetime_utc = itr_dt_utc->value.GetString();

			if (itr_level != l.MemberEnd() && itr_level->value.IsString())
			    level.level = itr_level->value.GetString();

			levels.emplace_back(level);
		    }
		}
		
		sleep_item.levels = levels;
	    }

	    if (itr_dates != v.MemberEnd() && itr_dates->value.IsArray())
	    {
		for (const auto& d : itr_dates->value.GetArray())
		{
		    if (d.IsString())
		    {
			sleep_item.dates.emplace_back(d.GetString());
		    }
		}
	    }

	    auto idx1 = sleep_item.dates.size() == 2 ? DateIdx(sleep_item.dates[0]) : DateIdx();
	    auto idx2 = sleep_item.dates.size() == 2 ? DateIdx(sleep_item.dates[1]) : DateIdx();
	    if (idx1.is_valid() && idx2.is_valid())
		this->sleep[idx1] = sleep_item;
	    else
		this->errors.emplace_back("JSON error: dates are no valid or are not dates.");
	}
	else
	{
	    this->errors.emplace_back(
		"JSON error: it expects an object with sleep information.");
	}
    }

    return true;
}

Activity& Activity::operator+=(const Activity& rhs)
{
    if (this->zone_info.empty())
	this->zone_info = rhs.zone_info;
    if (this->username.empty())
	this->username = rhs.username;
    if (this->sport_profile_name.empty())
	this->sport_profile_name = rhs.sport_profile_name;
    if (this->sport.empty())
	this->sport = rhs.sport;
    if (this->sub_sport.empty())
	this->sub_sport = rhs.sub_sport;
    this->start_lat_lon = rhs.start_lat_lon;
    this->end_lat_lon = rhs.end_lat_lon;
    if (this->start_time_utc.empty())
	this->start_time_utc = rhs.start_time_utc;
    this->total_elapsed_time += rhs.total_elapsed_time;
    this->total_timer_time += rhs.total_timer_time;
    this->total_distance += rhs.total_distance;
    this->avg_speed = this->avg_speed > 0 ?
	(this->avg_speed + rhs.avg_speed) / 2 : rhs.avg_speed;
    this->max_speed = this->max_speed > rhs.max_speed ?
	this->max_speed : rhs.max_speed;
    this->avg_cadence = this->avg_cadence > 0 ?
	(this->avg_cadence + rhs.avg_cadence) / 2 : rhs.avg_cadence;
    this->max_cadence = this->max_cadence > rhs.max_cadence ?
	this->max_cadence : rhs.max_cadence;
    this->avg_running_cadence = this->avg_running_cadence > 0 ?
	(this->avg_running_cadence + rhs.avg_running_cadence) / 2 : rhs.avg_running_cadence;
    this->max_running_cadence = this->max_running_cadence > rhs.max_running_cadence ?
	this->max_running_cadence : rhs.max_running_cadence;
    this->total_strides += rhs.total_strides;
    this->total_calories += rhs.total_calories;
    this->total_ascent += rhs.total_ascent;
    this->total_descent += rhs.total_descent;
    this->avg_temperature = this->avg_temperature > 0 ?
	(this->avg_temperature + rhs.avg_temperature) / 2 : rhs.avg_temperature;
    this->max_temperature = this->max_temperature > rhs.max_temperature ?
	this->max_temperature : rhs.max_temperature;
    this->min_temperature = this->min_temperature < rhs.min_temperature ?
	this->min_temperature : rhs.min_temperature;
    this->avg_respiration_rate = this->avg_respiration_rate > 0 ?
	(this->avg_respiration_rate + rhs.avg_respiration_rate) / 2 : rhs.avg_respiration_rate;
    this->max_respiration_rate = this->max_respiration_rate > rhs.max_respiration_rate ?
	this->max_respiration_rate : rhs.max_respiration_rate;
    this->min_respiration_rate = this->min_respiration_rate < rhs.min_respiration_rate ?
	this->min_respiration_rate : rhs.min_respiration_rate;
    this->training_load_peak += rhs.training_load_peak;
    this->total_training_effect += rhs.total_training_effect;
    this->total_anaerobic_training_effect += rhs.total_anaerobic_training_effect;

    return *this;
}

Activity operator+(Activity lhs, const Activity& rhs)
{
    lhs += rhs;
    return lhs;
}

bool ActivitiesData::load(const rapidjson::Document& document)
{
    const auto data = document.FindMember("data");
    if (data == document.MemberEnd() || !data->value.IsArray())
    {
	return false;
    }

    for (const auto& v : data->value.GetArray())
    {
	if (v.IsObject())
	{
	    auto itr_zone_info = v.FindMember("zone_info");
	    auto itr_username = v.FindMember("username");
	    auto itr_session = v.FindMember("session");

	    Activity activity{};

	    if (itr_zone_info != v.MemberEnd() && itr_zone_info->value.IsString())
	    {
		activity.zone_info = itr_zone_info->value.GetString();
	    }

	    if (itr_username != v.MemberEnd() && itr_username->value.IsString())
	    {
		activity.username = itr_username->value.GetString();
	    }

	    if (itr_session != v.MemberEnd() && itr_session->value.IsObject())
	    {
		auto itr_spn = itr_session->value.FindMember("sport_profile_name");
		auto itr_s = itr_session->value.FindMember("sport");
		auto itr_ss = itr_session->value.FindMember("sub_sport");
		auto itr_splat = itr_session->value.FindMember("start_position_lat");
		auto itr_splon = itr_session->value.FindMember("start_position_lon");
		auto itr_eplat = itr_session->value.FindMember("end_position_lat");
		auto itr_eplon = itr_session->value.FindMember("end_position_lon");
		auto itr_st = itr_session->value.FindMember("start_time");
		auto itr_et = itr_session->value.FindMember("total_elapsed_time");
		auto itr_tt = itr_session->value.FindMember("total_timer_time");
		auto itr_td = itr_session->value.FindMember("total_distance");
		auto itr_eas = itr_session->value.FindMember("enhanced_avg_speed");
		auto itr_as = itr_session->value.FindMember("avg_speed");
		auto itr_ems = itr_session->value.FindMember("enhanced_max_speed");
		auto itr_ms = itr_session->value.FindMember("max_speed");
		auto itr_ac = itr_session->value.FindMember("avg_cadence");
		auto itr_mc = itr_session->value.FindMember("max_cadence");
		auto itr_arc = itr_session->value.FindMember("avg_running_cadence");
		auto itr_mrc = itr_session->value.FindMember("max_running_cadence");
		auto itr_ts = itr_session->value.FindMember("total_strides");
		auto itr_tc = itr_session->value.FindMember("total_calories");
		auto itr_ta = itr_session->value.FindMember("total_ascent");
		auto itr_tde = itr_session->value.FindMember("total_descent");
		auto itr_at = itr_session->value.FindMember("avg_temperature");
		auto itr_mat = itr_session->value.FindMember("max_temperature");
		auto itr_mit = itr_session->value.FindMember("min_temperature");
		auto itr_earr = itr_session->value.FindMember("enhanced_avg_respiration_rate");
		auto itr_emaxrr = itr_session->value.FindMember("enhanced_max_respiration_rate");
		auto itr_eminrr = itr_session->value.FindMember("enhanced_min_respiration_rate");
		auto itr_tlp = itr_session->value.FindMember("training_load_peak");
		auto itr_tte = itr_session->value.FindMember("total_training_effect");
		auto itr_tate = itr_session->value.FindMember("total_anaerobic_training_effect");

		if (itr_spn != itr_session->value.MemberEnd() && itr_spn->value.IsString())
		    activity.sport_profile_name = itr_spn->value.GetString();

		if (itr_s != itr_session->value.MemberEnd() && itr_s->value.IsString())
		    activity.sport = itr_s->value.GetString();

		if (itr_ss != itr_session->value.MemberEnd() && itr_ss->value.IsString())
		    activity.sub_sport = itr_ss->value.GetString();

		if (itr_splat != itr_session->value.MemberEnd() && itr_splat->value.IsNumber() &&
		    itr_splon != itr_session->value.MemberEnd() && itr_splon->value.IsNumber())
		    activity.start_lat_lon = {itr_splat->value.GetFloat(), itr_splon->value.GetFloat()};

		if (itr_eplat != itr_session->value.MemberEnd() && itr_eplat->value.IsNumber() &&
		    itr_eplon != itr_session->value.MemberEnd() && itr_eplon->value.IsNumber())
		    activity.end_lat_lon = {itr_eplat->value.GetFloat(), itr_eplon->value.GetFloat()};

		if (itr_st != itr_session->value.MemberEnd() && itr_st->value.IsString())
		    activity.start_time_utc = itr_st->value.GetString();

		if (itr_et != itr_session->value.MemberEnd() && itr_et->value.IsNumber())
		    activity.total_elapsed_time = itr_et->value.GetFloat();

		if (itr_tt != itr_session->value.MemberEnd() && itr_tt->value.IsNumber())
		    activity.total_timer_time = itr_tt->value.GetFloat();

		if (itr_td != itr_session->value.MemberEnd() && itr_td->value.IsNumber())
		    activity.total_distance =itr_td->value.GetFloat();

		if (itr_eas != itr_session->value.MemberEnd() && itr_eas->value.IsNumber())
		    activity.avg_speed = itr_eas->value.GetFloat();
		else if (itr_as != itr_session->value.MemberEnd() && itr_as->value.IsNumber())
		    activity.avg_speed = itr_as->value.GetFloat();

		if (itr_ems != itr_session->value.MemberEnd() && itr_ems->value.IsNumber())
		    activity.max_speed = itr_ems->value.GetFloat();
		else if (itr_ms != itr_session->value.MemberEnd() && itr_ms->value.IsNumber())
		    activity.max_speed = itr_ms->value.GetFloat();

		if (itr_ac != itr_session->value.MemberEnd() && itr_ac->value.IsNumber())
		    activity.avg_cadence = itr_ac->value.GetFloat();

		if (itr_mc != itr_session->value.MemberEnd() && itr_mc->value.IsNumber())
		    activity.max_cadence = itr_mc->value.GetFloat();

		if (itr_arc != itr_session->value.MemberEnd() && itr_arc->value.IsNumber())
		    activity.avg_running_cadence = itr_arc->value.GetFloat();

		if (itr_mrc != itr_session->value.MemberEnd() && itr_mrc->value.IsNumber())
		    activity.max_running_cadence = itr_mrc->value.GetFloat();

		if (itr_ts != itr_session->value.MemberEnd() && itr_ts->value.IsNumber())
		    activity.total_strides = itr_ts->value.GetFloat();

		if (itr_tc != itr_session->value.MemberEnd() && itr_tc->value.IsNumber())
		    activity.total_calories = itr_tc->value.GetFloat();

		if (itr_ta != itr_session->value.MemberEnd() && itr_ta->value.IsNumber())
		    activity.total_ascent = itr_ta->value.GetFloat();

		if (itr_tde != itr_session->value.MemberEnd() && itr_tde->value.IsNumber())
		    activity.total_descent = itr_tde->value.GetFloat();

		if (itr_at != itr_session->value.MemberEnd() && itr_at->value.IsNumber())
		    activity.avg_temperature = itr_at->value.GetFloat();

		if (itr_mat != itr_session->value.MemberEnd() && itr_mat->value.IsNumber())
		    activity.max_temperature = itr_mat->value.GetFloat();

		if (itr_mit != itr_session->value.MemberEnd() && itr_mit->value.IsNumber())
		    activity.min_temperature = itr_mit->value.GetFloat();

		if (itr_earr != itr_session->value.MemberEnd() && itr_earr->value.IsNumber())
		    activity.avg_respiration_rate = itr_earr->value.GetFloat();

		if (itr_emaxrr != itr_session->value.MemberEnd() && itr_emaxrr->value.IsNumber())
		    activity.max_respiration_rate = itr_emaxrr->value.GetFloat();

		if (itr_eminrr != itr_session->value.MemberEnd() && itr_eminrr->value.IsNumber())
		    activity.min_respiration_rate = itr_eminrr->value.GetFloat();

		if (itr_tlp != itr_session->value.MemberEnd() && itr_tlp->value.IsNumber())
		    activity.training_load_peak = itr_tlp->value.GetFloat();

		if (itr_tte != itr_session->value.MemberEnd() && itr_tte->value.IsNumber())
		    activity.total_training_effect = itr_tte->value.GetFloat();

		if (itr_tate != itr_session->value.MemberEnd() && itr_tate->value.IsNumber())
		    activity.total_anaerobic_training_effect = itr_tate->value.GetFloat();
	    }

	    DateIdx idx{activity.start_time_utc};
	    if (!activity.start_time_utc.empty() && idx.is_valid())
		this->activities[idx] = activity;
	    else
		this->errors.emplace_back("JSON error: start time is not a valid date.");
	}
	else
	{
	    this->errors.emplace_back(
		"JSON error: it expects an object with activity information.");
	}
    }

    return true;
}

bool Error::has_error() const
{
    return this->error != ErrorType::Success || this->httplib_error != httplib::Error::Success;
}

std::string Error::error_to_string() const
{
    static const std::map<ErrorType, std::string> errorMessages = {
        {ErrorType::Success, "Success response."},
        {ErrorType::NotLoaded, "Error: response not loaded."},
        {ErrorType::Unknown, "Error: unknown."},
        {ErrorType::NotResponse, "Error: response not received."},
        {ErrorType::NotData, "Error: there is no data in the response received."},
        {ErrorType::Http100, "Error: 1xx code error."},
        {ErrorType::Http300, "Error: 3xx code error."},
        {ErrorType::Http400, "Error: 4xx code error."},
	{ErrorType::Http401, "Error: authentication error."},
        {ErrorType::Http500, "Error: 5xx code error."}
    };

    std::string result;

    auto errorString = errorMessages.find(error);
    if (errorString != errorMessages.end())
    {
	result = errorString->second;
    }

    if (httplib_error != httplib::Error::Success)
    {
	result += '\n';
	result += httplib::to_string(httplib_error);
    }

    if (result.empty())
    {
	return "Error: Unknown error.";
    }

    return result;
}

template <typename T>
void Result<T>::load(const httplib::Result &response)
{   
    if (!response)
    {
	this->error = Error(ErrorType::NotResponse);
	return;
    }

    this->status = response->status;

    if (response->status < 200)
    {
	this->error = Error(ErrorType::Http100, response.error());
	return;
    }

    if (response->status >= 300 && response->status < 400)
    {
	this->error = Error(ErrorType::Http300, response.error());
	return;
    }

    if (response->status == 401)
    {
	this->error = Error(ErrorType::Http401, response.error());
	return;
    }

    if (response->status > 401 && response->status < 500)
    {
	this->error = Error(ErrorType::Http400, response.error());
	return;
    }

    if (response->status >= 500)
    {
	this->error = Error(ErrorType::Http500, response.error());
	return;
    }

    rapidjson::Document document;
    document.Parse(response->body.c_str());
    this->data = std::make_unique<T>();
    bool is_valid = this->data->load(document);

    if (is_valid)
	this->error = Error(ErrorType::Success, response.error());
    else
	this->error = Error(ErrorType::NotData, response.error());
}

template <typename T>
void Result<T>::load(const T& newData)
{
    this->data = std::make_unique<T>(newData);
}

const Result<LoginData> Connection::login(
    const std::string& username, const std::string& password)
{
    httplib::Client client(this->host, this->port);
    std::stringstream credentials;
    rapidjson::Document document;

    credentials << "username=" << username << "&password=" << password;
    auto response = client.Post(
	"/auth/login", credentials.str(), "application/x-www-form-urlencoded");

    Result<LoginData> result;
    result.load(response);

    if (result.is_valid())
	this->token = result.get_data().access_token;

    return result;
}

void Connection::logout() { this->token = ""; }

bool Connection::has_token() const { return !this->token.empty(); }

const Result<UploadedFileData> Connection::do_post_for_file(
    httplib::Client& client, const std::filesystem::path& path) const
{
    try
    {
	// Open and read the file in a buffer.
	std::ifstream file(path.string(), std::ios::binary);
	std::stringstream buffer;
	buffer << file.rdbuf();
	file.close();

	// Send POST request with the file.
	httplib::MultipartFormDataItems items = {
	    { "files", buffer.str(), path.filename().string(), "application/octet-stream" },
	    { "zone", "Europe/Madrid", "", "" }
	};

	auto response = client.Post("/files/", items);
	Result<UploadedFileData> result;
	result.load(response);
	return result;
    }
    catch (const std::exception& e)
    {
	UploadedFileData ufd;
	ufd.file_path = path.string();
	ufd.accepted = false;
	ufd.errors = {e.what()};

        Result<UploadedFileData> result;
	result.load(ufd);

	return result;
    }
}

const std::vector<Result<UploadedFileData>> Connection::post_file(
    std::filesystem::path& path) const
{
    std::vector<Result<UploadedFileData>> results;

    httplib::Client client(this->host, this->port);
    client.set_bearer_token_auth(this->token);
    client.set_connection_timeout(0, 1000000); // 1000 milliseconds = 1 second
    client.set_read_timeout(10, 0); // 10 seconds
    client.set_write_timeout(10, 0); // 10 seconds

    if (std::filesystem::is_regular_file(path))
    {
	auto result = this->do_post_for_file(client, path);
	results.emplace_back(std::move(result));
    }
    else if (std::filesystem::is_directory(path))
    {
	for (const auto& e : std::filesystem::directory_iterator(path))
	{
	    auto p = e.path();
	    auto result = this->do_post_for_file(client, p);
	    results.emplace_back(std::move(result));
	}
    }

    return results;
}

const Result<StepsData> Connection::get_steps() const
{
    httplib::Client client(this->host, this->port);
    client.set_bearer_token_auth(this->token);
    auto response = client.Get("/monitorings/steps");
    
    Result<StepsData> result;
    result.load(response);
    return result;
}

const Result<SleepData> Connection::get_sleep() const
{
    httplib::Client client(this->host, this->port);
    client.set_bearer_token_auth(this->token);
    auto response = client.Get("/monitorings/sleep");
    
    Result<SleepData> result;
    result.load(response);
    return result;
}

const Result<ActivitiesData> Connection::get_activities() const
{
    httplib::Client client(this->host, this->port);
    client.set_bearer_token_auth(this->token);
    auto response = client.Get("/activities/");

    Result<ActivitiesData> result;
    result.load(response);
    return result;
}

} // namespace fitgalgo
