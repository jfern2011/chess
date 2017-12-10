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

	int get_hash_size() const;

	bool get_ponder() const;

	bool init();
	
	void set_debug(bool debug);

	void set_hash_size(int size);

	void set_ponder(bool on);

private:

	/**
	 * The value of the UCI "debug" option
	 */
	bool _debug;

	/**
	 * Size of the hash table, in MB
	 */
	int _hash_size;

	/**
	 * Log activity to a file descriptor
	 */
	Logger& _logger;

	/**
	 * True if pondering is enabled
	 */
	bool _ponder;

	/**
	 * The name of this module for logging
	 * purposes
	 */
	const std::string _name;
};

#endif
