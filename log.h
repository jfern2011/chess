#ifndef __LOG_H__
#define __LOG_H__

#include "util.h"

/**
 * A simple class used for logging activity. Any modules
 * that wish to write to the log must register with
 * \ref register_source
 */
class Logger
{

public:

	Logger();

	~Logger();

	bool assign_fd(int fd, bool nonblock=false);

	bool is_registered(const std::string& name) const;

	bool register_source(const std::string& name);

	bool write(const std::string& _source,
			   const char* format, ...);

private:

	/**
	 * The file descriptor to write to
	 */
	int _fd;

	/**
	 * The name of this module
	 */
	const std::string _name;

	/**
	 * A set of registered log sources
	 */
	Util::str_v _sources;
};

#endif
