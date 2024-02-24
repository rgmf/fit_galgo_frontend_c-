#include "api.h"
#include <memory>

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

short DateIdx::year() const
{
    return is_valid() ? static_cast<short>(std::atoi(datetime.substr(0, 4).c_str())) : -1;
}

short DateIdx::month() const
{
    return is_valid() ? static_cast<short>(std::atoi(datetime.substr(5, 2).c_str())) : -1;
}

short DateIdx::day() const
{
    return is_valid() ? static_cast<short>(std::atoi(datetime.substr(8, 2).c_str())) : -1;
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

    std::vector<UploadedFile> files{};
    
    for (const auto& v : data->value.GetArray())
    {
	if (v.IsObject())
	{
	    UploadedFile file{};

	    const auto id = v.FindMember("id");
	    const auto filename = v.FindMember("filename");
	    const auto accepted = v.FindMember("accepted");
	    const auto zip_filename = v.FindMember("zip_filename");
	    const auto errors = v.FindMember("errors");

	    if (id != v.MemberEnd() && id->value.IsString())
		file.id = id->value.GetString();

	    if (filename != v.MemberEnd() && filename->value.IsString())
		file.filename = filename->value.GetString();

	    if (accepted != v.MemberEnd() && accepted->value.IsBool())
		file.accepted = accepted->value.GetBool();

	    if (zip_filename != v.MemberEnd() && zip_filename->value.IsString())
		file.zip_filename = zip_filename->value.GetString();

	    if (errors != v.MemberEnd() && errors->value.IsArray())
		for (const auto& v : errors->value.GetArray())
		    if (v.IsString())
			file.errors.emplace_back(v.GetString());

	    this->uploaded_files.emplace_back(file);
	}
	else
	{
	    this->errors.emplace_back(
		"JSON error: it expects an object with file uploaded information.");
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

std::ostream& operator<<(std::ostream& os, ActivityType at)
{
    switch(at)
    {
    case ActivityType::GENERIC: os << "Generic"; break;
    case ActivityType::DISTANCE: os << "Distance"; break;
    case ActivityType::SPLITS: os << "Splits"; break;
    case ActivityType::SETS: os << "Sets"; break;
    default: os.setstate(std::ios_base::failbit);
    }
    return os;
}

ActivityType Activity::get_id() const
{
    return ActivityType::GENERIC;
}
/*
void SetsActivity::merge(const std::unique_ptr<Activity>& a)
{
    Activity::merge(a);

    if (a->get_id() != ActivityType::SETS)
    {
        const auto& sets = static_cast<SetsActivity*>(a.get())->sets;
        this->sets.insert(this->sets.end(), sets.cbegin(), sets.cend());
    }
}
*/

ActivityType SetsActivity::get_id() const
{
    return ActivityType::SETS;
}

/*
void SplitsActivity::merge(const std::unique_ptr<Activity>& a)
{
    Activity::merge(a);

    if (a->get_id() != ActivityType::SPLITS)
    {
        const auto& splits = static_cast<SplitsActivity*>(a.get())->splits;
        this->splits.insert(this->splits.end(), splits.cbegin(), splits.cend());
    }
}
*/

ActivityType SplitsActivity::get_id() const
{
    return ActivityType::SPLITS;
}

/*
void DistanceActivity::merge(const std::unique_ptr<Activity>& a)
{
    Activity::merge(a);

    if (a->get_id() != ActivityType::DISTANCE)
    {
	const auto& records = static_cast<DistanceActivity*>(a.get())->records;
	const auto& laps = static_cast<DistanceActivity*>(a.get())->laps;
	this->records.insert(this->records.end(), records.cbegin(), records.cend());
	this->laps.insert(this->laps.end(), laps.cbegin(), laps.cend());
    }
}
*/

ActivityType DistanceActivity::get_id() const
{
    return ActivityType::DISTANCE;
}

ActivitiesData::ActivitiesData(const ActivitiesData& other)
{
    activities.clear();
    for (const auto& [idx, a] : other.activities)
    {
	switch (a->get_id())
	{
	case ActivityType::DISTANCE:
	    activities[idx] = std::make_unique<DistanceActivity>(dynamic_cast<DistanceActivity&>(*a));
	    break;
	case ActivityType::SETS:
	    activities[idx] = std::make_unique<SetsActivity>(dynamic_cast<SetsActivity&>(*a));
	    break;
	case ActivityType::SPLITS:
	    activities[idx] = std::make_unique<SplitsActivity>(dynamic_cast<SplitsActivity&>(*a));
	    break;
	default:
	    activities[idx] = std::make_unique<Activity>(*a);
	    break;
	}
    }

    errors = other.errors;
}

ActivitiesData& ActivitiesData::operator=(const ActivitiesData& other)
{
    if (this != &other)
    {
	activities.clear();
	
	for (const auto& [idx, a] : other.activities)
	{
	    switch (a->get_id())
	    {
	    case ActivityType::DISTANCE:
		activities[idx] = std::make_unique<DistanceActivity>(dynamic_cast<DistanceActivity&>(*a));
		break;
	    case ActivityType::SETS:
		activities[idx] = std::make_unique<SetsActivity>(dynamic_cast<SetsActivity&>(*a));
		break;
	    case ActivityType::SPLITS:
		activities[idx] = std::make_unique<SplitsActivity>(dynamic_cast<SplitsActivity&>(*a));
		break;
	    default:
		activities[idx] = std::make_unique<Activity>(*a);
		break;
	    }
	}

	errors = other.errors;
    }
    return *this;
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
	    std::unique_ptr<Activity> activity;

	    auto itr_zone_info = v.FindMember("zone_info");
	    auto itr_username = v.FindMember("username");
	    auto itr_session = v.FindMember("session");
	    auto itr_splits = v.FindMember("splits");
	    auto itr_sets = v.FindMember("sets");
	    if (itr_splits != v.MemberEnd() && itr_splits->value.IsArray())
	    {
		float total_work_time = 0;
		activity = std::make_unique<SplitsActivity>();
		for (const auto& s : itr_splits->value.GetArray())
		{
		    if (s.IsObject())
		    {
			Split split{};
			
			auto itr_st = s.FindMember("split_type");
			auto itr_tet = s.FindMember("total_elapsed_time");
			auto itr_ttt = s.FindMember("total_timer_time");
			auto itr_sti = s.FindMember("start_time");
			auto itr_ah = s.FindMember("avg_hr");
			auto itr_mh = s.FindMember("max_hr");
			auto itr_tc = s.FindMember("total_calories");
			auto itr_d = s.FindMember("difficulty");
			auto itr_r = s.FindMember("result");
			auto itr_di = s.FindMember("discarded");
			
			if (itr_st != s.MemberEnd() && itr_st->value.IsString())
			    split.split_type = itr_st->value.GetString();
			if (itr_tet != s.MemberEnd() && itr_tet->value.IsFloat())
			    split.total_elapsed_time = itr_tet->value.GetFloat();
			if (itr_sti != s.MemberEnd() && itr_sti->value.IsFloat())
			    split.start_time = itr_sti->value.GetFloat();
			if (itr_ah != s.MemberEnd() && itr_ah ->value.IsInt())
			    split.avg_hr = itr_ah->value.GetInt();
			if (itr_mh != s.MemberEnd() && itr_mh ->value.IsInt())
			    split.max_hr = itr_mh->value.GetInt();
			if (itr_tc != s.MemberEnd() && itr_tc->value.IsInt())
			    split.total_calories = itr_tc->value.GetInt();
			if (itr_d != s.MemberEnd() && itr_d->value.IsInt())
			    split.difficulty = itr_d->value.GetInt();
			if (itr_di != s.MemberEnd() && itr_di->value.IsInt())
			    split.result = SplitResult::DISCARDED;
			if (itr_r != s.MemberEnd() && itr_r->value.IsInt())
			{
			    if (itr_r->value.GetInt() == 2) 
				split.result = SplitResult::ATTEMPTED;
			    else if (itr_r->value.GetInt() == 3) 
				split.result = SplitResult::COMPLETED;
			}
			if (itr_ttt != s.MemberEnd() && itr_ttt->value.IsFloat())
			{
			    split.total_timer_time = itr_ttt->value.GetFloat();
			    if (split.result == SplitResult::ATTEMPTED || split.result == SplitResult::COMPLETED)
				total_work_time += split.total_timer_time;
			}

			(static_cast<SplitsActivity*>(activity.get()))->splits.emplace_back(split);
		    }
		}
		if (total_work_time > 0)
		    activity->total_work_time = total_work_time;
	    }
	    else if (itr_sets != v.MemberEnd() && itr_sets->value.IsArray())
	    {
		float total_work_time = 0;
		activity = std::make_unique<SetsActivity>();
		for (const auto& s : itr_sets->value.GetArray())
		{
		    if (s.IsObject())
		    {
			Set set{};

			auto itr_t = s.FindMember("timestamp");
			auto itr_st = s.FindMember("set_type");
			auto itr_d = s.FindMember("duration");
			auto itr_r = s.FindMember("repetition");
			auto itr_w = s.FindMember("weight");
			auto itr_sti = s.FindMember("start_time");
			auto itr_c = s.FindMember("category");
			auto itr_sc = s.FindMember("sub_category");
			auto itr_wdu = s.FindMember("weight_display_unit");
			auto itr_mi = s.FindMember("message_index");
			auto itr_wsi = s.FindMember("wkt_step_index");

			if (itr_t != s.MemberEnd() && itr_t->value.IsFloat())
			    set.timestamp = itr_t->value.GetFloat();
			if (itr_st != s.MemberEnd() && itr_st->value.IsString())
			{
			    auto it = std::find(std::begin(set_type_names), std::end(set_type_names), itr_st->value.GetString());
			    if (it != std::end(set_type_names))
			    {
				std::size_t idx = std::distance(std::begin(set_type_names), it);
				set.set_type = SetType(idx);
			    }
			}
			if (itr_d != s.MemberEnd() && itr_d->value.IsFloat())
			{
			    set.duration = itr_d->value.GetFloat();
			    if (set.set_type == SetType::ACTIVE)
				total_work_time += set.duration;
			}
			if (itr_r != s.MemberEnd() && itr_r->value.IsInt())
			    set.repetitions = itr_r->value.GetInt();
			if (itr_w != s.MemberEnd() && itr_w->value.IsFloat())
			    set.weight = itr_w->value.GetFloat();
			if (itr_sti != s.MemberEnd() && itr_sti->value.IsFloat())
			    set.start_time = itr_sti->value.GetFloat();
			if (itr_c != s.MemberEnd() && itr_c->value.IsArray())
			{
			    for (const auto& v : itr_c->value.GetArray())
			    {
				if (v.IsInt())
				    set.category.emplace_back(std::to_string(v.GetInt()));
				else if (v.IsString())
				    set.category.emplace_back(v.GetString());
			    }
			}
			if (itr_sc != s.MemberEnd() && itr_sc->value.IsArray())
			{
			    for (const auto& v : itr_sc->value.GetArray())
			    {
				if (v.IsInt())
				    set.category_subtype.emplace_back(std::to_string(v.GetInt()));
				else if (v.IsString())
				    set.category_subtype.emplace_back(v.GetString());
			    }
			}
			if (itr_wdu != s.MemberEnd() && itr_wdu->value.IsString())
			    set.weight_display_unit = itr_wdu->value.GetString();
			if (itr_mi != s.MemberEnd() && itr_mi->value.IsInt())
			    set.message_index = itr_mi->value.GetInt();
			if (itr_wsi != s.MemberEnd() && itr_wsi->value.IsInt())
			    set.wkt_step_index = itr_wsi->value.GetInt();

			(static_cast<SetsActivity*>(activity.get()))->sets.emplace_back(set);
		    }
		}
		if (total_work_time > 0)
		    activity->total_work_time = total_work_time;
	    }
	    else
	    {
		activity = std::make_unique<DistanceActivity>();
	    }

	    if (itr_zone_info != v.MemberEnd() && itr_zone_info->value.IsString())
	    {
		activity->zone_info = itr_zone_info->value.GetString();
	    }

	    if (itr_username != v.MemberEnd() && itr_username->value.IsString())
	    {
		activity->username = itr_username->value.GetString();
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
		    activity->sport_profile_name = itr_spn->value.GetString();
		
		if (itr_s != itr_session->value.MemberEnd() && itr_s->value.IsString())
		    activity->sport = itr_s->value.GetString();

		if (itr_ss != itr_session->value.MemberEnd() && itr_ss->value.IsString())
		    activity->sub_sport = itr_ss->value.GetString();

		if (itr_splat != itr_session->value.MemberEnd() && itr_splat->value.IsNumber() &&
		    itr_splon != itr_session->value.MemberEnd() && itr_splon->value.IsNumber())
		    activity->start_lat_lon = {itr_splat->value.GetFloat(), itr_splon->value.GetFloat()};

		if (itr_eplat != itr_session->value.MemberEnd() && itr_eplat->value.IsNumber() &&
		    itr_eplon != itr_session->value.MemberEnd() && itr_eplon->value.IsNumber())
		    activity->end_lat_lon = {itr_eplat->value.GetFloat(), itr_eplon->value.GetFloat()};

		if (itr_st != itr_session->value.MemberEnd() && itr_st->value.IsString())
		    activity->start_time_utc = itr_st->value.GetString();

		if (itr_et != itr_session->value.MemberEnd() && itr_et->value.IsNumber())
		    activity->total_elapsed_time = itr_et->value.GetFloat();

		if (itr_tt != itr_session->value.MemberEnd() && itr_tt->value.IsNumber())
		{
		    activity->total_timer_time = itr_tt->value.GetFloat();
		    if (!activity->total_work_time.has_value())
			activity->total_work_time = activity->total_timer_time;
		}

		if (itr_td != itr_session->value.MemberEnd() && itr_td->value.IsNumber())
		    activity->total_distance =itr_td->value.GetFloat();

		if (itr_eas != itr_session->value.MemberEnd() && itr_eas->value.IsNumber())
		    activity->avg_speed = itr_eas->value.GetFloat();
		else if (itr_as != itr_session->value.MemberEnd() && itr_as->value.IsNumber())
		    activity->avg_speed = itr_as->value.GetFloat();

		if (itr_ems != itr_session->value.MemberEnd() && itr_ems->value.IsNumber())
		    activity->max_speed = itr_ems->value.GetFloat();
		else if (itr_ms != itr_session->value.MemberEnd() && itr_ms->value.IsNumber())
		    activity->max_speed = itr_ms->value.GetFloat();

		if (itr_ac != itr_session->value.MemberEnd() && itr_ac->value.IsNumber())
		    activity->avg_cadence = itr_ac->value.GetFloat();

		if (itr_mc != itr_session->value.MemberEnd() && itr_mc->value.IsNumber())
		    activity->max_cadence = itr_mc->value.GetFloat();

		if (itr_arc != itr_session->value.MemberEnd() && itr_arc->value.IsNumber())
		    activity->avg_running_cadence = itr_arc->value.GetFloat();

		if (itr_mrc != itr_session->value.MemberEnd() && itr_mrc->value.IsNumber())
		    activity->max_running_cadence = itr_mrc->value.GetFloat();

		if (itr_ts != itr_session->value.MemberEnd() && itr_ts->value.IsNumber())
		    activity->total_strides = itr_ts->value.GetFloat();

		if (itr_tc != itr_session->value.MemberEnd() && itr_tc->value.IsNumber())
		    activity->total_calories = itr_tc->value.GetFloat();

		if (itr_ta != itr_session->value.MemberEnd() && itr_ta->value.IsNumber())
		    activity->total_ascent = itr_ta->value.GetFloat();

		if (itr_tde != itr_session->value.MemberEnd() && itr_tde->value.IsNumber())
		    activity->total_descent = itr_tde->value.GetFloat();

		if (itr_at != itr_session->value.MemberEnd() && itr_at->value.IsNumber())
		    activity->avg_temperature = itr_at->value.GetFloat();

		if (itr_mat != itr_session->value.MemberEnd() && itr_mat->value.IsNumber())
		    activity->max_temperature = itr_mat->value.GetFloat();

		if (itr_mit != itr_session->value.MemberEnd() && itr_mit->value.IsNumber())
		    activity->min_temperature = itr_mit->value.GetFloat();

		if (itr_earr != itr_session->value.MemberEnd() && itr_earr->value.IsNumber())
		    activity->avg_respiration_rate = itr_earr->value.GetFloat();

		if (itr_emaxrr != itr_session->value.MemberEnd() && itr_emaxrr->value.IsNumber())
		    activity->max_respiration_rate = itr_emaxrr->value.GetFloat();

		if (itr_eminrr != itr_session->value.MemberEnd() && itr_eminrr->value.IsNumber())
		    activity->min_respiration_rate = itr_eminrr->value.GetFloat();

		if (itr_tlp != itr_session->value.MemberEnd() && itr_tlp->value.IsNumber())
		    activity->training_load_peak = itr_tlp->value.GetFloat();

		if (itr_tte != itr_session->value.MemberEnd() && itr_tte->value.IsNumber())
		    activity->total_training_effect = itr_tte->value.GetFloat();

		if (itr_tate != itr_session->value.MemberEnd() && itr_tate->value.IsNumber())
		    activity->total_anaerobic_training_effect = itr_tate->value.GetFloat();
	    }

	    DateIdx idx{activity->start_time_utc};
	    if (!activity->start_time_utc.empty() && idx.is_valid())
		this->activities[idx] = std::move(activity);
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
Result<T>::Result()
{
    error = Error();
    status = {};
    data = std::unique_ptr<T>();
}

template <typename T>
Result<T>::Result(const Result<T>& other)
{
    error = other.error;
    status = other.status;
    data = std::make_unique<T>(*other.data);
}

template <typename T>
Result<T>::Result(Result<T>&& other) noexcept
{
    error = std::move(other.error);
    status = other.status;
    data = std::move(other.data);
}

template <typename T>
Result<T>& Result<T>::operator=(const Result<T>& other)
{
    if (this != &other)
    {
	error = other.error;
	status = other.status;
	data = other.data ? std::make_unique<T>(*other.data) : std::make_unique<T>();
    }
    return *this;
}

template <typename T>
Result<T>& Result<T>::operator=(const Result<T>&& other) noexcept
{
    if (this != &other)
    {
	error = other.error;
	status = other.status;
	data = other.data ? std::make_unique<T>(*other.data) : std::make_unique<T>();
    }
    return *this;
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

    Result<LoginData> result{};
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
	UploadedFile uf;
	uf.filename = path.string();
	uf.accepted = false;
	UploadedFileData ufd;
	ufd.uploaded_files.emplace_back(uf);
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

template class Result<LoginData>;

} // namespace fitgalgo
