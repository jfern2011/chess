#include "settings.h"

/**
 * Constructor
 *
 * @param[in] logger The Logger that this component can write
 *                   diagnostics to
 */
EngineSettings::EngineSettings(Logger& logger)
	: _debug(false),
	  _logger(logger), _name("EngineSettings")
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
 */
bool EngineSettings::get_debug() const
{
	return _debug;
}

/**
 * Initialize
 */
bool EngineSettings::init()
{
	AbortIfNot(_logger.register_source(_name),
		false);

	return true;
}

/**
 * Set the value of the UCI debug option
 */
void EngineSettings::set_debug(bool debug)
{
	std::string val;
	Util::to_string<bool>(debug, val);

	_logger.write(_name, "setting debug to %s.\n",
		val.c_str());

	_debug = debug;
}
