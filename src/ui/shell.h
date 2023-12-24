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
    explicit ShellSteps(const StepsData& steps_data)
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
    explicit ShellSleep(const SleepData& sleep_data)
	: stats(Stats<Sleep>(sleep_data.sleep)) {}
    void loop() const;
};

class ShellActivities
{
private:
    Stats<Activity> stats;

    void dashboard() const;
    void month_stats(const ushort& year) const;
    void week_stats(const ushort& year, const ushort& month) const;

public:
    explicit ShellActivities(const ActivitiesData& activities_data)
	: stats(Stats<Activity>(activities_data.activities)) {}
    void loop() const;
};

} // namespace fitgalgo

#endif // _ES_RGMF_UI_SHELL_H
