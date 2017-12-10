#include "settings.h"

/**
 * Constructor
 *
 * @param[in] logger The Logger that this component can write
 *                   diagnostics to
 */
EngineSettings::EngineSettings(Logger& logger)
	: _debug(false),
	  _hash_size(0),
	  _logger(logger),
	  _name("EngineSettings"),
	  _ponder(false)
{
}

/**
 * Destructor
 */
EngineSettings::~EngineSettings()
{
}

/**
 * Get the current value of the UCI debug option
 *
 * @return True if debugging is enabled
 */
bool EngineSettings::get_debug() const
{
	return _debug;
}

/**
 * Get the combined size of all hash tables
 *
 * @return The size, in MB
 */
int EngineSettings::get_hash_size() const
{
	return _hash_size;
}

/**
 * Get the pondering flag
 *
 * @return True if pondering is enabled
 */
bool EngineSettings::get_ponder() const
{
	return _ponder;
}

/**
 * Initialize.
 *
 * @return True on success
 */
bool EngineSettings::init()
{
	AbortIfNot(_logger.register_source(_name),
		false);

	return true;
}

/**
 * Set the value of the UCI debug option
 *
 * @param[in] debug True or false
 */
void EngineSettings::set_debug(bool debug)
{
	std::string val;
	Util::to_string<bool>(debug, val);

	_logger.write(_name, "setting debug to %s.\n",
		val.c_str());

	_debug = debug;
}

/**
 * Set the total size allocated to hash tables
 *
 * @param[in] bytes The size, in MB
 */
void EngineSettings::set_hash_size(int size)
{
	std::string val;
	Util::to_string<int>(size, val);

	_logger.write(_name, "setting hash tables to %s MB.\n",
		val.c_str());

	_hash_size = size;
}

/**
 * Enable or disable engine pondering
 *
 * @param[in] on True to enable
 */
void EngineSettings::set_ponder(bool on)
{
	if (on)
		_logger.write(_name, "pondering enabled. \n");
	else
		_logger.write(_name, "pondering disabled.\n");

	_ponder = on;
}
