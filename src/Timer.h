#ifndef __TIMER_H__
#define __TIMER_H__

#include "chess4.h"

namespace Chess
{
    /**
     * A simple profiler
     */
    class Timer final
    {
        const int64 billion = 1e9;

    public:

        Timer();

        ~Timer();

        int64 elapsed() const;

        int64 start();

        int64 stop();

    private:

        int64 _t_now() const;

        /**
         * True if the timer is running
         */
        bool _is_running;

        /**
         * Last start->stop period (ns)
         */
        int64 _time_used;

        /**
         * The start time
         */
        int64 _tstart;
    };
}

#endif
