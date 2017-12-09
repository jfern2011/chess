#ifndef __SETTINGS_H__
#define __SETTINGS_H__

#include "log.h"

/**
 * A class that maintains the engine configuration parameters,
 * settable by the UCI or xBoard interfaces
 */
class EngineSettings
{

public:

	EngineSettings(Logger& logger);

	~EngineSettings();

	bool get_debug() const;

	bool init();
	
	void set_debug(bool debug);

private:

	/**
	 * The value of the UCI "debug" option
	 */
	bool _debug;

	/**
	 * Log activity to a file descriptor
	 */
	Logger& _logger;

	/**
	 * The name of this module for logging
	 * purposes
	 */
	const std::string _name;
};

#endif
