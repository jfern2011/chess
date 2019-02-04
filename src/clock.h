#ifndef __CLOCK_H__
#define __CLOCK_H__

#include <cstdint>
#include <time.h>

class Clock
{
	const static std::int64_t nano_per_sec = 1000000000;

public:

	Clock()
	{
	}

	~Clock()
	{
	}

	static std::int64_t get_monotonic_time()
	{
		struct timespec ts;

		if (::clock_gettime(CLOCK_MONOTONIC, &ts))
			return 0;

		std::int64_t sec = ts.tv_sec;
		std::int64_t ns = ts.tv_nsec;

		return (sec * nano_per_sec + ns);
	}

private:

};

#endif
