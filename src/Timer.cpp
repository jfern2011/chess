#include <time.h>

#include "Timer.h"

#ifdef _WIN64
#include <windows.h>
#endif

namespace Chess
{
	/**
	 * Constructor
	 */
	Timer::Timer() : _is_running(false), _time_used(0), _tstart(0)
	{
	}

	/**
	 * Destructor
	 */
	Timer::~Timer()
	{
	}

	/**
	 * If running, this is the elapsed time so far. Otherwise, this
	 * is the last start->stop interval
	 *
	 * @return The elapsed time, in nanoseconds
	 */
	int64 Timer::elapsed() const
	{
		if (_is_running)
		{
			return _t_now() - _tstart;
		}

		return _time_used;
	}

	/**
	 * Start the timer
	 *
	 * @return The absolute start time, in nanoseconds
	 */
	int64 Timer::start()
	{
		_is_running = true; _tstart = _t_now();
		return _tstart;
	}

	/**
	 * Stop the timer
	 *
	 * @return  The absolute stop time, in nanoseconds
	 */
	int64 Timer::stop()
	{
		const int64 t_stop = _t_now();

		_is_running = false;
		_time_used = t_stop - _tstart;

		return t_stop;
	}

	/**
	 * Get the current monotonic time
	 *
	 * @return The monotonic time, in nanoseconds
	 */
	int64 Timer::_t_now() const
	{
#ifdef _WIN64
		LARGE_INTEGER now, freq;

		QueryPerformanceFrequency(&freq);
		QueryPerformanceCounter(&now);

		/*
		 * Convert number of clock ticks to nanoseconds
		 */
		return now.QuadPart * billion / freq.QuadPart;
#else
		struct timespec ts;
		if (::clock_gettime(CLOCK_MONOTONIC_RAW, &ts) < 0)
			return -1;

		return ts.tv_sec * billion
				+ ts.tv_nsec;
#endif
	}
}
