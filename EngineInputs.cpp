#include "EngineInputs.h"

/**
 * Constructor
 *
 * @param[in] logger The Logger that this component will write
 *                   diagnostics to
 */
EngineInputs::EngineInputs(Logger& logger)
	: _debug(false),
	  _hash_size(0),
	  _is_init(false),
	  _logger(logger),
	  _name("EngineInputs"),
	  _ponder(false),
	  _position(nullptr)
{
}

/**
 * Destructor
 */
EngineInputs::~EngineInputs()
{
	if (_position) delete _position;
}

/**
 * Get the current value of the UCI debug option
 *
 * @return True if debugging is enabled
 */
bool EngineInputs::get_debug() const
{
	return _debug;
}

/**
 * Get the combined size of all hash tables
 *
 * @return The size, in MB
 */
int EngineInputs::get_hash_size() const
{
	return _hash_size;
}

/**
 * Get the pondering flag
 *
 * @return True if pondering is enabled
 */
bool EngineInputs::get_ponder() const
{
	return _ponder;
}

/**
 * Get the current position
 *
 * @return The internal position
 */
const Position* EngineInputs::get_position() const
{
	return _position;
}

/**
 * Initialize.
 *
 * @param[in] pos Initialize with this position
 *
 * @return True on success
 */
bool EngineInputs::init(const Position& pos)
{
	AbortIf(_is_init, false);
	AbortIfNot(_logger.register_source(_name),
		false);

	_position = new Position(pos);

	_is_init = true;
	return true;
}

/**
 * Set the value of the UCI debug option
 *
 * @param[in] debug True or false
 */
void EngineInputs::set_debug(bool debug)
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
void EngineInputs::set_hash_size(int size)
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
void EngineInputs::set_ponder(bool on)
{
	if (on)
		_logger.write(_name, "pondering enabled. \n");
	else
		_logger.write(_name, "pondering disabled.\n");

	_ponder = on;
}

/**
 * Set the internal copy of the position to \a pos
 *
 * @param[in] pos The new position
 *
 * @return True on success
 */
bool EngineInputs::set_position( const Position& pos )
{
	AbortIfNot(_is_init, false);

	*_position = pos;
	return true;
}
