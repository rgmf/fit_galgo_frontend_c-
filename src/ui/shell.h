#ifndef _ES_RGMF_UI_SHELL_H
#define _ES_RGMF_UI_SHELL_H 1

#include "../core/api.h"
#include "../core/stats.h"

namespace fitgalgo
{

class ShellSteps
{
private:
    Stats<Steps> stats;

    void year_stats() const;
    void month_stats(const ushort& year) const;
    void week_stats(const ushort& year, const ushort& month) const;

public:
    ShellSteps(const StepsData& steps_data)
	: stats(Stats<Steps>(steps_data.steps)) {}
    void loop() const;
};

class ShellSleep
{
private:
    Stats<Sleep> stats;

    void year_stats() const;
    void month_stats(const ushort& year) const;
    void week_stats(const ushort& year, const ushort& month) const;
    
public:
    ShellSleep(const SleepData& sleep_data)
	: stats(Stats<Sleep>(sleep_data.sleep)) {}
    void loop() const;
};

// for (auto& [k, v] : result.get_data().sleep)
    // {
    // 	std::cout << k << ": "
    // 		  << "Overall sleep score: "
    // 		  << v.assessment.overall_sleep_score << " | "
    // 		  << "Sleep quality score: "
    // 		  << v.assessment.sleep_quality_score << " | "
    // 		  << "Awakenings count: "
    // 		  << v.assessment.awakenings_count << " | "
    // 		  << v.assessment.combined_awake_score << " | "
    // 		  << v.assessment.awake_time_score << " | "
    // 		  << v.assessment.awakenings_count_score << " | "
    // 		  << v.assessment.deep_sleep_score << " | "
    // 		  << v.assessment.sleep_duration_score << " | "
    // 		  << v.assessment.light_sleep_score << " | "
    // 		  << v.assessment.sleep_recovery_score << " | "
    // 		  << v.assessment.rem_sleep_score << " | "
    // 		  << v.assessment.sleep_restlessness_score << " | "
    // 		  << v.assessment.interruptions_score << " | "
    // 		  << v.assessment.average_stress_during_sleep
    // 		  << std::endl;
    // 	std::cout << "Number of levels: " << v.levels.size() << std::endl << std::endl;
    // }
}

#endif // _ES_RGMF_UI_SHELL_H
