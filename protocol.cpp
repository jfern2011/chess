#include "output.h"
#include "protocol2.h"

/**
 * Constructor
 *
 * @param[in] logger The Logger that this component can write
 *                   diagnostics to
 */
Protocol::Protocol(Logger& logger)
	: settings(logger),
	  _cmd(),
	  _is_init( false ),
	  _logger(logger)
{
}

/**
 * Destructor
 */
Protocol::~Protocol()
{
}

/**
 * Get a reference to the internal commanding interface which
 * handles user commands
 *
 * @return The commanding interface
 */
CommandInterface& Protocol::get_cmd_interface()
{
	return _cmd;
}

/**
 * Construct a Universal Chess Interface
 */
UCI::UCI(Logger& logger)
	: Protocol(logger),
	  _name("UCI"), _options()
{
}

/**
 * Destructor
 */
UCI::~UCI()
{
	for (size_t i = 0; i < _options.size(); i++ )
		delete _options[i];
}

/**
 * Install the commands defined in this protocol
 *
 * @return True on success
 */
bool UCI::_init_commands()
{
	AbortIf(_is_init, false);

	AbortIfNot(_cmd.install<UCI>("debug", *this, &UCI::debug),
		false);

	AbortIfNot(_cmd.install<UCI>("uci", *this, &UCI::uci),
		false);

	AbortIfNot(_cmd.install<UCI>("isready", *this, &UCI::isready),
		false);

	AbortIfNot(_cmd.install<UCI>("ucinewgame", *this,
		&UCI::ucinewgame), false);

	AbortIfNot(_cmd.install<UCI>("position", *this, &UCI::position),
		false);

	return true;
}

/**
 * Define the parameters that can be set by the user via the GUI
 *
 * @return True on success
 */
bool UCI::_init_options()
{
	AbortIf(_is_init, false);

	/*
	 * Note: Assign an engine updater to every option that
 	 * needs one. This allows the value field of the
 	 * "setoption" command to make its way to the engine's
 	 * internal settings
 	 */

	{
		/*
	 	 * Hash table size
	 	 */
		auto opt = new option<int>("Hash", // Name
								   "spin", // Type
								   16,     // Default (MB)
								   0,      // Min
								   65536); // Max
		AbortIfNot(opt, false);
		AbortIfNot(opt->assign_updater(
			settings, &EngineSettings::set_hash_size), false);

		_options.push_back(opt);
	}

	{
		/*
	 	 * Pondering
	 	 */
		auto opt = new option<bool>("Ponder",
									"check",
									false);

		AbortIfNot(opt, false);
		AbortIfNot(opt->assign_updater(
			   settings, &EngineSettings::set_ponder), false);

		_options.push_back(opt);
	}

	return true;
}

/**
 * The handler for the "debug" command
 *
 * @param[in] _state Set the debug option to this. Should either
 *                   be "on" or "off"
 *
 * @return True on success
 */
bool UCI::debug(const std::string& _state)
{
	const std::string state =
		Util::to_lower(Util::trim(_state));

	if (state != "on" && state != "off")
	{
		_logger.write(_name,
			"unable to set debug state to '%s'", state.c_str());
		return true;
	}

	if (state == "on")
		settings.set_debug(true );
	else
		settings.set_debug(false);

	return true;
}

/**
 * Get an iterator to the option with the specified name
 *
 * @param[in] name The name of the option
 *
 * @return An iterator to the option give by \a name, or
 *         vector::end if not found
 */
std::vector<UCI::option_base*>::iterator
	UCI::find_option(const std::string& name)
{
	for (auto iter = _options.begin(), end = _options.end();
		 iter != end; ++iter)
	{
		const std::string _name = (*iter)->name;

		if ( Util::to_lower(_name) == Util::to_lower(name) )
			return iter;
	}

	return _options.end();
}

/**
 * Initialize this interface
 *
 * @param[in] fd The file descriptor on which to read user commands
 *
 * @return True on success
 */
bool UCI::init(int fd)
{
	/*
	 * Register the engine settings component with
	 * the Logger
	 */
	AbortIfNot(settings.init(), false);

	AbortIfNot(_cmd.init(fd),
		false);

	AbortIfNot(_init_commands(), false);
	AbortIfNot(_init_options(),  false);

	AbortIfNot(_logger.register_source(_name),
		false);

	_is_init = true;
	return true;
}

/**
 * The handler for the "isready" UCI command
 *
 * @return True on success
 */
bool UCI::isready(const std::string&) const
{
	AbortIfNot(Output::to_stdout("readyok\n"),
		false);

	return true;
}

/**
 * The handler for the "position" UCI command
 *
 * @param[in] _args The arguments passed in from the command
 *                  interface
 *
 * @return True on success
 */
bool UCI::position(const std::string& _args) const
{
	return true;
}

/**
 * The handler for the "register" command
 *
 * @return True on success
 */
bool UCI::register_engine(const std::string&) const
{
	return true;
}

/**
 * The command handler for the UCI "setoption" user command
 *
 * @return True on success
 */
bool UCI::setoption(const std::string& _args)
{
	Util::str_v args;
	Util::split(_args, args);

	if (args.size() < 2)
	{
		_logger.write(_name, "too few inputs '%s'\n",  _args.c_str() );

		return false;
	}

	auto iter = find_option(Util::trim(args[1]));

	if (iter == _options.end())
	{
		_logger.write(_name, "unknown option '%s'\n", args[1].c_str());

		return false;
	}

	const option_base* option = *iter;

	if (option->type == "button")
	{
		// code for button options
		return true;
	}

	/*
	 * If we got here, there should also be a value specified
	 * for this option:
	 */
	if (args.size() != 4
		|| Util::to_lower(Util::trim(args[0])) != "name"
		|| Util::to_lower(Util::trim(args[2])) != "value")
	{
		_logger.write(_name, "invalid command syntax '%s'\n",
					  _args.c_str());
		return false;
	}

	if (!option->update(args[3]))
	{
		_logger.write(
				_name, "failed to set option '%s' to '%s'\n",
				option->name.c_str(),
				args[3].c_str());

		return false;
	}

	return true;
}

/**
 * Sniff the file descriptor for user commands, dispatching
 * handlers as needed
 *
 * @return True on success
 */
bool UCI::sniff()
{
	AbortIfNot(_is_init, false);

	return _cmd.poll();
}

/**
 * Handles the "uci" command. Specifically, this replies to the GUI
 * with the chess engine ID info and a list of engine parameters
 * that can be set manually by the user. This list is complete when
 * we send the "uciok" message
 *
 * @return True on success
 */
bool UCI::uci(const std::string&) const
{
	AbortIfNot(_is_init, false);

	static const std::string name("Bender");

	AbortIfNot(Output::to_stdout("id name %s\n" , name.c_str()),
		false);
	AbortIfNot(Output::to_stdout("id author Jason Fernandez\n"),
		false);

	for (auto iter = _options.begin(), end = _options.end();
		 iter != end; ++iter)
	{
		const auto& ptr = *iter;

		switch (ptr->inputs)
		{
		case 5:
			AbortIfNot(Output::to_stdout(
				"option name %s type %s default %s min %s max %s",
				ptr->name.c_str(),
				ptr->type.c_str(),
				ptr->default_to_string().c_str(),
				ptr->min_to_string().c_str(),
				ptr->max_to_string().c_str()), false);
			break;
		case 3:
			AbortIfNot(Output::to_stdout(
				"option name %s type %s default %s",
				ptr->name.c_str(),
				ptr->type.c_str(),
				ptr->default_to_string().c_str()), false);
			break;
		case 2:
			AbortIfNot(Output::to_stdout(
				"option name %s type %s default %s",
				ptr->name.c_str(),
				ptr->type.c_str()), false);
			break;
		default:
			// We should never get here:
			Abort(false);
		}

		/*
		 * Send the predefined values (if any) to standard
		 * output:
		 */
		Util::str_v vars;
		AbortIfNot( ptr->predefs_to_string(vars),
			false);

		std::string predefs = "";
		for (size_t i = 0; i < vars.size(); i++)
		{
			predefs += " var " + vars[i];
		}

		AbortIfNot(Output::to_stdout("%s\n",
			predefs.c_str()), false);
	}

	/*
	 * Send the "uciok" footer:
	 */
	AbortIfNot(Output::to_stdout("uciok\n"),
		false);

	return true;
}

/**
 * Handles the "ucinewgame" command.
 *
 * @return True on success
 */
bool UCI::ucinewgame(const std::string&) const
{
	return true;
}

xBoard::xBoard(Logger& logger)
	: Protocol(logger),
	  _is_init(false),
	  _logger(logger),
	  _name("xBoard")
{
}

xBoard::~xBoard()
{
}

bool xBoard::init(int fd)
{
	/*
	 * Register the engine settings component with
	 * the Logger
	 */
	AbortIfNot(settings.init(), false);

	AbortIfNot(_cmd.init(fd),
		false);

	AbortIfNot(_logger.register_source(_name),
		false);

	_is_init = true;
	return true;
}

bool xBoard::sniff()
{
	return true;
}

Console::Console(Logger& logger)
	: Protocol(logger),
	  _is_init(false),
	  _logger(logger),
	  _name("Console")
{
}

Console::~Console()
{
}

bool Console::init(int fd)
{
	/*
	 * Register the engine settings component with
	 * the Logger
	 */
	AbortIfNot(settings.init(), false);

	AbortIfNot(_cmd.init(fd),
		false);

	AbortIfNot(_logger.register_source(_name),
		false);

	_is_init = true;
	return true;
}

bool Console::sniff()
{
	return true;
}
