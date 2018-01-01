#include <cstdarg>

#include "Buffer.h"
#include "output2.h"

/**
 * Constructor
 *
 * @param[in] name   The name of this software component
 * @param[in] logger Writes to the log file
 */
OutputWriter::OutputWriter(const std::string& name, Logger& logger)
	: _logger(logger), _name(name)
{
}

/**
 * Destructor
 */
OutputWriter::~OutputWriter()
{
}

/**
 * Write a formatted string to standard output. This flushes
 * stdout to minimize buffering
 *
 * @param[in] format  A format string. See std::printf() for
 *                    details
 *
 * @return True on success
 */
bool OutputWriter::write(const char* format, ...) const
{
	static Buffer<char,1024> output;

	va_list args;

	va_start(args, format);

	AbortIf(std::vsprintf(output, format, args) < 0,
		false);

	va_end(args);

	std::string msg(output);
	if (msg.back() == '\n') msg.pop_back();

	AbortIfNot(_logger.write(_name,
			"sending output string '%s'\n", msg.c_str()),
		false);

	std::printf("%s", (char*)output);
	std::fflush(stdout);

	return true;
}
