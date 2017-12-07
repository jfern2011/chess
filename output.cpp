#include <cstdarg>

#include "abort.h"

namespace Output
{
	/**
	 * Write a formatted string to standard output. This flushes
	 * stdout to ensure it isn't kept buffered
	 *
	 * @return True on success
	 */
	bool to_stdout( const char* format, ... )
	{
		va_list args;

		va_start(args, format);

		AbortIf(std::vprintf(format, args) < 0,
			false);

		va_end(args);

		std::fflush(stdout);
		return true;
	}
}
