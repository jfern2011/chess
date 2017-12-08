#include <cstdio>

#include "settings.h"
#include "util.h"

EngineSettings::EngineSettings(Logger& logger)
	: _debug(false),
	  _logger(logger), _name("EngineSettings")
{
}

EngineSettings::~EngineSettings()
{
}

bool EngineSettings::get_debug() const
{
	return _debug;
}

bool EngineSettings::init()
{
	AbortIfNot(_logger.register_source(_name),
		false);

	return true;
}

void EngineSettings::set_debug(bool debug)
{
	std::string val;
	Util::to_string<bool>(debug, val);

	_logger.write(_name, "setting debug to %s.\n",
		val.c_str());

	_debug = debug;
}
