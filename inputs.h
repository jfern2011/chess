#ifndef __INPUTS_H__
#define __INPUTS_H__

#include "log.h"

class EngineInputs
{

public:

	EngineInputs(Logger& logger);

	~EngineInputs();

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
