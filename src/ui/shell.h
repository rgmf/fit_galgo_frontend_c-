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
}

#endif // _ES_RGMF_UI_SHELL_H
