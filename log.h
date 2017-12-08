#ifndef __LOG_H__
#define __LOG_H__

#include "util.h"

class Logger
{

public:

	Logger();

	~Logger();

	bool assign_fd(int fd);

	bool is_registered(const std::string& name) const;

	bool register_source(const std::string& name);

	bool write(const std::string& _source,
			   const char* format, ...);

private:

	int _fd;

	Util::str_v _sources;
};

#endif
