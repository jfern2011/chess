#include "cmd.h"

/**
 * Constructor
 */
CommandInterface::CommandInterface()
	: _cmds(),
	  _is_init(false), _wes()
{
}

/**
 * Destructor
 */
CommandInterface::~CommandInterface()
{
}

/**
 * Initialize.
 *
 * @param [in] fd The file descriptor on which to listen for
 *                commands
 *
 * @return True on success
 */
bool CommandInterface::init(int fd)
{
	AbortIf(_is_init, false);

	AbortIfNot(_wes.assign_fd(fd), false);

	AbortIfNot(_wes.attach_reader(*this,
		&CommandInterface::handle_command), false);

	_is_init = true;
	return true;
}

/**
 * Return the flag indicating whether or not \ref init()
 * has been called
 *
 * @return True if initialized
 */
bool CommandInterface::is_init() const
{
	return _is_init;
}

/**
 * Register a new command whose handler is a static function
 *
 * @param[in] name The name of this command
 * @param[in] func A function pointer to the command handler
 *
 * @return True on success
 */
bool CommandInterface::install(const std::string& name,
							   bool(*func)(const std::string&))
{
	signal_t* sig =
		new Signal::fcn_ptr<bool,const std::string&>(func);

	AbortIfNot(_install(name, sig), false);

    return true;
}

/**
 * Check whether a particular command has been registered. Note
 * that leading/trailing whitespace is ignored
 *
 * @param[in] name The name of the command
 *
 * @return True if this command is currently registered
 */
bool CommandInterface::is_installed(const std::string& name) const
{
	std::string _name = Util::to_lower(Util::trim(name));
	return _cmds.find(_name) != _cmds.end();
}

/**
 * Poll the underlying file descriptor for inputs, dispatching
 * command handlers as needed
 *
 * @return True on success
 */
bool CommandInterface::poll()
{
	WriteEventSink::err_code code =
					_wes.read( std::string("\n") );

	AbortIf(code != WriteEventSink::SUCCESS &&
			code != WriteEventSink::NO_DATA,false);

	return true;
}

/**
 * Common code to create a new command
 *
 * @param[in] _name The name of this command
 * @param[in] sig   Signal that, when raised, invokes the handler
 *
 * @return True on success
 */
bool CommandInterface::_install(const std::string& _name,
								signal_t* sig)
{
	AbortIfNot(_is_init, false);
	const std::string name = Util::to_lower(Util::trim(_name));

	AbortIf(sig == nullptr || !sig->is_connected(),
		false);

	AbortIf(name.size() == 0, false);

	if (is_installed(name))
	{
		std::printf("duplicate command '%s'.\n", name.c_str());
		Abort(false);
	}

	_cmds[name] = std::move(
		cmd_info(sig,_cmds.size(), name));

	return true;
}

/**
 * The callback routine dispatched by the event sink. Handles
 * a single command
 *
 * @param[in] _input File descriptor input buffer
 * @param[in] size   Size of \a _input
 *
 * @return True on success
 */
bool CommandInterface::handle_command(const char* _input,
									  size_t size)
{
	AbortIf(size == 0, false);

	std::string temp  = std::string(_input, size);
	std::string input =
		std::move(Util::trim(temp));

	Util::str_v tokens;
		Util::split( input,tokens );

	AbortIf(tokens.size() < 1, false);

	auto iter =
		_cmds.find(Util::to_lower(tokens[0]));

	if (iter == _cmds.end())
	{
		std::cout << "Error (unknown command): " << tokens[0]
			<< std::endl;
		return true;
	}

	const cmd_info& cmd = iter->second;

	if (tokens.size() == 1)
		return 
			cmd.handler->raise("");
	else
	{
		tokens.erase(tokens.begin());
		return
			cmd.handler->raise(Util::build_string(tokens,
				" "));
	}
}
