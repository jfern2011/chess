#ifndef __CLOCK_H__
#define __CLOCK_H__

#include <time.h>

#include "types.h"

#define NS_PER_SEC 1000000000

class Clock
{

public:

	Clock()
	{
	}

	~Clock()
	{
	}

	static int64 get_monotonic_time()
	{
		struct timespec ts;

		if (clock_gettime(CLOCK_MONOTONIC, &ts))
			return 0;

		int64 sec = ts.tv_sec;
		int64 ns = ts.tv_nsec;

		return
			sec * NS_PER_SEC + ns;
	}

private:

};

#endif