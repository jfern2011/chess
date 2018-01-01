#ifndef __LOG_H__
#define __LOG_H__

#include "util.h"

/**
 * @class Logger
 *
 * A simple class used for logging activity. Any modules
 * that wish to write to the log register with \ref
 * register_source so that if something erroneous occurs
 * we'll know the source
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
			   const char* format, ...) const;

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
