#include <algorithm>
#include <cstdarg>
#include <fcntl.h>
#include <unistd.h>

#include "Buffer.h"
#include "log.h"
#include "output.h"

/**
 * Constructor
 */
Logger::Logger()
	: _fd(-1), _name("Logger"), _sources()
{
}

/**
 * Destructor
 */
Logger::~Logger()
{
}

/**
 * Assign a file descriptor to write to. Note that this can be called
 * any number of times
 *
 * @param[in] fd       The file descriptor
 * @param[in] nonblock If true, then logging will not be performed if
 *                     writing would block
 *
 * @return True on success
 */
bool Logger::assign_fd(int fd, bool nonblock)
{
	AbortIf(fd < 0, false);
	_fd = fd;

	if (nonblock)
	{
		/*
		 * Set the file descriptor to perform non-blocking
		 * writes:
		 */
		const int flags = ::fcntl(_fd, F_GETFL, 0);
		AbortIf(flags == -1, false);

		AbortIf(::fcntl(_fd, F_SETFL, flags | O_NONBLOCK),
			false);
	}

	return true;
}

/**
 * Determine whether a specific log source has been registered
 *
 * @param[in] name The source name
 *
 * @return True if \a name has been registered
 */
bool Logger::is_registered(const std::string& name) const
{
	auto iter = std::find(_sources.begin(),
						  _sources.end(), name);

	return iter != _sources.end();
}

/**
 * Register a new log source. External code must register here
 * to \ref write() to the log
 *
 * @param[in] name The source name
 *
 * @return True on success
 */
bool Logger::register_source(const std::string& name)
{
	std::string source = Util::trim(name);

	AbortIf(source.empty(), false);

	if (is_registered(source))
	{
		Output::to_stdout("duplicate source '%s' \n",
						  name.c_str());
		Abort(false);
	}

	_sources.push_back(source);

	return true;
}

/**
 * Attempt to write to the log file
 *
 * @param[in] _source The name of the registered source attempting
 *                    to write to the log
 * @param[in] format  This is just like std::printf()
 *
 * @return True on success
 */
bool Logger::write(const std::string& _source,
				   const char* format, ...)
{
	AbortIf(_fd == -1, false);

	const std::string source = Util::trim(_source);

	static const int buf_size = 1024;
	static Buffer<char,buf_size> buf;

	int nchars = 0;
	if (!is_registered(source))
	{
		/*
		 * Deny strangers access:
		 */
		nchars = std::snprintf(buf, buf_size,
			"%s: unknown source '%s' attempted to access the log.\n",
			_name.c_str(), _source.c_str());
	}
	else
	{
		const std::string format_s = source + ": " + format;

		va_list args;

		va_start(args, format);

		nchars =
			std::vsnprintf( buf , buf_size, format_s.c_str(), args );

		va_end(args);
	}

	/*
	 * Nothing we can do if this fails:
	 */
	if (nchars > 0)
		::write(_fd, buf, nchars);
	
	return true;
}
