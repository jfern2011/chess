#include "output.h"
#include "protocol2.h"

/**
 * Constructor
 *
 * @param[in] name    The name of this component
 * @param[in] _inputs The EngineInputs that we'll set via user
 *                    commands
 * @param[in] logger  The Logger that this component can write
 *                    diagnostics to
 */
Protocol::Protocol(const std::string& name, EngineInputs& _inputs,
		Logger& logger)
	: StateMachineClient(name),
	  inputs(_inputs),
	  _cmd(logger),
	  _is_init(false),
	  _logger(logger),
	  _name(name)
{
}

/**
 * Destructor
 */
Protocol::~Protocol()
{
}

/**
 * Get a reference to the internal commanding interface, which
 * handles user commands
 *
 * @return The commanding interface
 */
CommandInterface& Protocol::get_cmd_interface()
{
	return _cmd;
}

/**
 * Get the name of this software component
 *
 * @return The component name
 */
std::string Protocol::get_name() const
{
	return _name;
}

/**
 * Construct a Universal Chess Interface
 *
 * @param[in] inputs A reference to the user inputs to forward
 *                   to the search algorithm
 * @param[in] logger Log activity to this
 */
UCI::UCI(EngineInputs& inputs, Logger& logger)
	: Protocol("UCI", inputs, logger),
	  _options()
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

	AbortIfNot(_cmd.install<UCI>("go", *this, &UCI::go),
		false);

	AbortIfNot(_cmd.install<UCI>("quit", *this, &UCI::quit),
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
			inputs, &EngineInputs::set_hash_size), false);

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
			   inputs, &EngineInputs::set_ponder), false);

		_options.push_back(opt);
	}

	{
		/*
	 	 * Multi PV
	 	 */
		auto opt = new option<int>("MultiPV",
								   "spin",
								   1, 1, MAX_PV);

		AbortIfNot(opt, false);
		AbortIfNot(opt->assign_updater(
			   inputs, &EngineInputs::set_multipv), false);

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
		inputs.set_debug(true );
	else
		inputs.set_debug(false);

	return true;
}

/**
 * Grab an iterator to the option with the specified name
 *
 * @param[in] name The name of the option
 *
 * @return An iterator to the option given by \a name, or
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
 * The handler for the "go" command
 *
 * @param [in] _args Arguments passed in from the command interface
 *
 * @return True on success
 */
bool UCI::go(const std::string& _args)
{
	Util::str_v args;

	Util::split(_args, args);

	bool searchmoves = false;
	std::string moves = "";
	for (auto iter = args.begin(), end = args.end(); iter != end;
		 ++iter)
	{
		bool parse_error = false;

		if (*iter == "searchmoves")
		{
			searchmoves = true;
			continue;
		}
		else if (*iter == "ponder")
		{
			searchmoves = false;

			inputs.set_ponder(true);
		}
		else if (*iter == "wtime")
		{
			searchmoves = false;

			int ms;
			if (iter + 1 == end || !Util::str_to_i32(*(iter+1), 10, ms)
				|| !inputs.set_time(ms, WHITE))
			{
				parse_error = true;
			}
		}
		else if (*iter == "btime")
		{
			searchmoves = false;

			int ms;
			if (iter + 1 == end || !Util::str_to_i32(*(iter+1), 10, ms)
				|| !inputs.set_time(ms, BLACK))
			{
				parse_error = true;
			}
		}
		else if (*iter == "winc")
		{
			searchmoves = false;

			int ms;
			if (iter + 1 == end || !Util::str_to_i32(*(iter+1), 10, ms)
				|| !inputs.set_increment(ms, WHITE))
			{
				parse_error = true;
			}
		}
		else if (*iter == "binc")
		{
			searchmoves = false;

			int ms;
			if (iter + 1 == end || !Util::str_to_i32(*(iter+1), 10, ms)
				|| !inputs.set_increment(ms, BLACK))
			{
				parse_error = true;
			}
		}
		else if (*iter == "movestogo")
		{
			searchmoves = false;

			int n;
			if (iter + 1 == end || !Util::str_to_i32(*(iter+1), 10, n )
				|| !inputs.set_movestogo(n))
			{
				parse_error = true;
			}
		}
		else if (*iter == "depth")
		{
			searchmoves = false;

			int n;
			if (iter + 1 == end || !Util::str_to_i32(*(iter+1), 10, n )
				|| !inputs.set_depth(n))
			{
				parse_error = true;
			}
		}
		else if (*iter == "nodes")
		{
			searchmoves = false;

			int n;
			if (iter + 1 == end || !Util::str_to_i32(*(iter+1), 10, n )
				|| !inputs.set_node_limit(n))
			{
				parse_error = true;
			}
		}
		else if (*iter == "mate")
		{
			searchmoves = false;

			int n;
			if (iter + 1 == end || !Util::str_to_i32(*(iter+1), 10, n )
				|| !inputs.set_mate_depth(n))
			{
				parse_error = true;
			}
		}
		else if (*iter == "movetime")
		{
			searchmoves = false;

			int ms;
			if (iter + 1 == end || !Util::str_to_i32(*(iter+1), 10, ms)
				|| !inputs.set_movetime(ms))
			{
				parse_error = true;
			}
		}
		else if (*iter == "infinite")
		{
			searchmoves = false;

			parse_error = !inputs.set_depth(MAX_PLY);
		}

		if (parse_error)
		{
			_logger.write(_name, "[%s] unable to set parameter '%s'\n",
				__FUNCTION__, iter->c_str());
			return false;
		}

		if (searchmoves)
		{
			moves += (*iter + " ");
		}
	}

	if (!inputs.searchmoves(moves))
	{
		moves = Util::trim( moves );
		_logger.write(_name,"[%s] unable to set searchmoves to '%s'\n",
			__FUNCTION__, moves.c_str());
		return false;
	}

	/*
	 * Request a state transition to StateMachine::init_search
	 */
	AbortIfNot(transition_sig.is_connected(),
		false);

	return transition_sig.raise(
		_name, StateMachine::init_search, false);
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
	if (_args.empty())
	{
		_logger.write(_name, "no arguments passed to [%s]\n",
			__FUNCTION__);
		return false;
	}

	Util::str_v args, parts;
	Util::split(_args, parts, "moves");

	Util::split(parts[0], args);

	if (args[0] != "startpos" && args[0] != "fen")
	{
		_logger.write(_name,
			"expected \"startpos\" or \"fen\", got \"%s\"\n",
			args[0].c_str());
		return false;
	}

	Position pos(*inputs.get_position());

	if (args[0] == "startpos")
	{
		AbortIfNot(pos.reset(true), false);
	}
	else
	{
		if (args.size() < 2)
		{
			_logger.write(_name,"missing FEN string\n");
			return false;
		}

		args.erase(args.begin());

		std::string fen = Util::build_string(args, " ");

		if (!pos.reset(fen, true))
		{
			_logger.write(_name, "invalid FEN \"%s\"\n",
				fen.c_str());
			return false;
		}
	}

	/*
	 * The position was reset, now play any moves passed
	 * in by the GUI
	 */
	Util::str_v moves;

	if (parts.size() > 1) Util::split(parts[1], moves);

	for (size_t i = 0; i < moves.size(); i++)
	{
		const int partial =
			Util::parse_coordinate(moves[i]);

		if (partial == 0)
		{
			_logger.write(_name, "invalid move '%s'",
				moves[i].c_str());
			return false;
		}

		const int promote = PROMOTE(partial);
		const int to = TO(partial);
		const int from = FROM(partial );

		const piece_t moved =
			pos.piece_on(FROM(partial));

		if (moved == INVALID)
		{
			_logger.write(_name, "invalid move '%s'",
				moves[i].c_str());
			return false;
		}

		piece_t captured =
			pos.piece_on( TO(partial) );

		if (captured == INVALID && moved == PAWN
			&& FILE(from) != FILE(to))
		{
			captured = PAWN; // en passant
		}

		int move = pack(captured, from, moved,
						promote, to);

		AbortIfNot(pos.make_move(move), false);
	}

	AbortIfNot(inputs.set_position(pos),
		false);

	return true;
}

/**
 * Send UCI-specific messages to the GUI that signal the end of
 * a search
 *
 * @param[in] outputs Send from these outputs
 *
 * @return True on success
 */
bool UCI::postsearch(EngineOutputs& outputs)
{
	AbortIfNot(transition_sig.is_connected(),
		false);

	outputs.update();

	auto bestmove = outputs["bestmove"];
	AbortIfNot(bestmove, false);

	auto ponder   = outputs["ponder"];
	AbortIfNot(ponder, false);

	std::string out = "bestmove " + bestmove->get();
	if (inputs.get_ponder())
		out += " ponder " + ponder->get();

	Output::to_stdout("%s\n", out.c_str());

	transition_sig.raise( _name, StateMachine::idle,
		false);

	outputs.mark_stale();

	return true;
}

/**
 * The handler for the "quit" command
 *
 * @return True on success
 */
bool UCI::quit(const std::string&)
{
	AbortIfNot(transition_sig.raise(_name, StateMachine::exiting,
		false), false);

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
 * Send periodic info from the currently running search
 * to the GUI
 *
 * @param[in] outputs Send these outputs as a string
 *
 * @return True on success
 */
bool UCI::send_periodics(const EngineOutputs& outputs)
{
	std::string output = "info ";

	

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

		switch (ptr->display_type)
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
 * Handles the "ucinewgame" command. This will request a state
 * transition to \ref StateMachine::idle
 *
 * @return True on success
 */
bool UCI::ucinewgame(const std::string&)
{
	AbortIfNot( transition_sig.is_connected(),
		false);

	return transition_sig.raise(
			_name, StateMachine::idle, false);
}

xBoard::xBoard(EngineInputs& inputs, Logger& logger)
	: Protocol("xBoard", inputs, logger),
	  _is_init(false),
	  _logger(logger)
{
}

xBoard::~xBoard()
{
}

bool xBoard::init(int fd)
{
	AbortIfNot(_cmd.init(fd),
		false);

	AbortIfNot(_logger.register_source(_name),
		false);

	_is_init = true;
	return true;
}

bool xBoard::postsearch(EngineOutputs& outputs)
{
	return true;
}

bool xBoard::send_periodics(const EngineOutputs& outputs)
{
	return true;
}

bool xBoard::sniff()
{
	return true;
}

Console::Console(EngineInputs& inputs, Logger& logger)
	: Protocol("Console", inputs, logger),
	  _is_init(false),
	  _logger(logger)
{
}

Console::~Console()
{
}

bool Console::init(int fd)
{
	AbortIfNot(_cmd.init(fd),
		false);

	AbortIfNot(_logger.register_source(_name),
		false);

	_is_init = true;
	return true;
}

bool Console::postsearch(EngineOutputs& outputs)
{
	return true;
}

bool Console::send_periodics(const EngineOutputs& outputs)
{
	return true;
}

bool Console::sniff()
{
	return true;
}
