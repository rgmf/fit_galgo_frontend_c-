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
    explicit Shell() : connection{} {}
    void loop();
};

class ShellStats
{
protected:
    ushort day;
    ushort year;
    ushort month;

    virtual void all_times_stats() const = 0;
    virtual void year_stats() const = 0;
    virtual void month_stats() const = 0;
    virtual void item_by_item_stats() const = 0;

public:
    void loop();
};

class ShellSteps : public ShellStats
{
private:
    StepsData data;

    void all_times_stats() const override;
    void year_stats() const override;
    void month_stats() const override;
    void item_by_item_stats() const override;

public:
    explicit ShellSteps(const StepsData& steps_data);
};

class ShellSleep : public ShellStats
{
private:
    SleepData data;

    void all_times_stats() const override;
    void year_stats() const override;
    void month_stats() const override;
    void item_by_item_stats() const override;

public:
    explicit ShellSleep(const SleepData& sleep_data);
};

class ShellActivities: public ShellStats
{
private:
    ActivitiesData data;
    Connection connection;

    void all_times_stats() const override;
    void year_stats() const override;
    void month_stats() const override;
    void item_by_item_stats() const override;

    void print_calendar() const;

public:
    explicit ShellActivities(const ActivitiesData& activities_data, const Connection& conn);
};

} // namespace fitgalgo

#endif // _ES_RGMF_UI_SHELL_H
