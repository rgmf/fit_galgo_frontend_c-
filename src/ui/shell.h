#ifndef _ES_RGMF_UI_SHELL_H
#define _ES_RGMF_UI_SHELL_H 1

#include "../core/api.h"

namespace fitgalgo
{

class Shell
{
private:
    Connection connection;

    inline bool login();
    inline void upload_path() const;
    inline void steps() const;
    inline void sleep() const;
    inline void activities() const;

public:
    explicit Shell(const std::string& host, const ushort& port)
	: connection(host, port) {}
    void loop();
};

class ShellSteps
{
private:
    StepsData data;
    ushort year;
    ushort month;

    void all_times_stats() const;
    void year_stats() const;
    void month_stats() const;

public:
    explicit ShellSteps(const StepsData& steps_data);
    void loop();
};

class ShellSleep
{
private:
    SleepData data;
    ushort year;
    ushort month;

    void all_times_stats() const;
    void year_stats() const;
    void month_stats() const;
    
public:
    explicit ShellSleep(const SleepData& sleep_data);
    void loop();
};

class ShellActivities
{
private:
    ActivitiesData data;
    ushort year;
    ushort month;

    void all_times_stats() const;
    void year_stats() const;
    void month_stats() const;
    void print_calendar() const;

public:
    explicit ShellActivities(const ActivitiesData& activities_data);
    void loop();
};

} // namespace fitgalgo

#endif // _ES_RGMF_UI_SHELL_H
