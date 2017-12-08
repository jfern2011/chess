#include <algorithm>
#include <cstdarg>
#include <fcntl.h>
#include <unistd.h>

#include "Buffer.h"
#include "log.h"
#include "output.h"

Logger::Logger()
	: _fd(-1), _sources()
{
}

Logger::~Logger()
{
}

bool Logger::assign_fd(int fd)
{
	AbortIf(fd < 0, false);
	_fd = fd;

	/*
	 * Set the file descriptor to perform non-blocking
	 * writes:
	 */
	const int flags = ::fcntl(_fd, F_GETFL, 0);
	AbortIf(flags == -1, false);

	AbortIf(::fcntl(_fd, F_SETFL, flags | O_NONBLOCK),
		false);

	return true;
}

bool Logger::is_registered(const std::string& name) const
{
	auto iter = std::find(_sources.begin(),
						  _sources.end(), name);

	return iter != _sources.end();
}

bool Logger::register_source(const std::string& name)
{
	std::string source =
		Util::to_lower(Util::trim(name));

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

bool Logger::write(const std::string& _source,
				   const char* format, ...)
{
	AbortIf(_fd == -1, false);

	const std::string source =
		Util::to_lower(Util::trim(_source));

	static const int buf_size = 1024;
	static Buffer<char,buf_size> buf;

	int n = 0;
	if (!is_registered(source))
	{
		n = std::snprintf(buf, buf_size,
						  "unknown source '%s' attempted "
						  "to access the log!\n",
			              _source.c_str());
	}
	else
	{
		va_list args;

		va_start(args, format);

		int n = std::snprintf(buf,buf_size, format, args);

		va_end(args);
	}

	/*
	 * Nothing we can do if this fails:
	 */
	if (n > 0)
		::write(_fd, buf, n);
	
	return true;
}
