#include "protocol2.h"

/**
 * Constructor
 *
 * @param[in] name    The name of this component
 * @param[in] tables  The global databases used throughout the
 *                    engine
 * @param[in] _inputs The EngineInputs that we'll set via user
 *                    commands
 * @param[in] logger  The Logger that this component will send
 *                    diagnostics to
 */
Protocol::Protocol(const std::string& name, const DataTables& tables,
				   EngineInputs& _inputs, Logger& logger)

	: StateMachineClient(name), OutputWriter(name, logger),
	  inputs(_inputs),
	  _cmd(logger),
	  _is_init(false),
	  _logger(logger),
	  _myname(name),
	  _tables(tables)
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
	return _myname;
}

/**
 * Construct a Universal Chess Interface
 *
 * @param[in] tables A reference to the global DataTables used
 *                   throughout the engine
 * @param[in] inputs A reference to the user inputs to forward
 *                   to the search algorithm
 * @param[in] logger Log activity to this
 */
UCI::UCI(const DataTables& tables, EngineInputs& inputs,
	     Logger& logger)
	: Protocol("UCI", tables, inputs, logger),
	  _bestmove_token(-1),
	  _options(),
	  _ponder_token(-1)
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

	AbortIfNot(_cmd.install<UCI>("isready", *this, &UCI::isready),
		false);

	AbortIfNot(_cmd.install<UCI>("uci", *this, &UCI::uci),
		false);

	AbortIfNot(_cmd.install<UCI>("debug", *this, &UCI::debug),
		false);

	AbortIfNot(_cmd.install<UCI>("setoption", *this, &UCI::setoption),
		false);

	AbortIfNot(_cmd.install<UCI>("register", *this,
		&UCI::register_engine), false);

	AbortIfNot(_cmd.install<UCI>("ucinewgame", *this,
		&UCI::ucinewgame), false);

	AbortIfNot(_cmd.install<UCI>("position", *this, &UCI::position),
		false);

	AbortIfNot(_cmd.install<UCI>("go", *this, &UCI::go),
		false);

	AbortIfNot(_cmd.install<UCI>("stop", *this, &UCI::stop),
		false);

	AbortIfNot(_cmd.install<UCI>("ponderhit", *this, &UCI::ponderhit),
		false);

	AbortIfNot(_cmd.install<UCI>("quit", *this, &UCI::quit),
		false);

	return true;
}

/**
 * Create the parameters that can be set by the user via the GUI
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
		auto opt = new Spin("Hash", // Name
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
		auto opt = new Check("Ponder", false);

		AbortIfNot(opt, false);
		AbortIfNot(opt->assign_updater(
			   inputs, &EngineInputs::set_ponder), false);

		_options.push_back(opt);
	}

	{
		/*
	 	 * Multi PV
	 	 */
		auto opt = new Spin("MultiPV", 1, 1, MAX_PV);

		AbortIfNot(opt, false);
		AbortIfNot(opt->assign_updater(
			   inputs, &EngineInputs::set_multipv), false);

		_options.push_back(opt);
	}

	return true;
}

/**
 * Initialize post-search outputs that we'll send to the GUI
 *
 * @param[in] search The source of our outputs
 *
 * @return True on success
 */
bool UCI::_init_outputs(const Search* search)
{
	AbortIfNot(search, false);

	auto& outputs =
		search->get_outputs();

	AbortIf((_bestmove_token = outputs.get_id("bestmove")) < 0,
		false);
	AbortIf((_ponder_token = outputs.get_id("ponder")) < 0,
		false);
	AbortIf((_pv_token = outputs.get_id("pv")) < 0,
		false);
	AbortIf((_search_depth_token = outputs.get_id("search_depth")) < 0,
		false);
	AbortIf((_nodes_searched_token = outputs.get_id("nodes_searched")) < 0,
		false);
	AbortIf((_search_time_token = outputs.get_id("search_time")) < 0,
		false);
	AbortIf((_num_lines_token = outputs.get_id("nlines")) < 0,
		false);
	AbortIf((_search_score_token = outputs.get_id("search_score")) < 0,
		false);
	AbortIf((_mate_in_token = outputs.get_id("mate_in")) < 0,
		false);
	AbortIf((_fail_hi_token = outputs.get_id("fail_hi")) < 0,
		false);
	AbortIf((_fail_lo_token = outputs.get_id("fail_lo")) < 0,
		false);
	AbortIf((_current_move_token = outputs.get_id("current_move")) < 0,
		false);
	AbortIf((_current_movenumber_token = outputs.get_id("current_move_number")) < 0,
		false);
	AbortIf((_hash_usage_token = outputs.get_id("hash_usage")) < 0,
		false);
	AbortIf((_nps_token = outputs.get_id("nps")) < 0,
		false);

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
		_logger.write(_myname,
			"unable to set debug state to '%s'\n", state.c_str());
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
			
			inputs.set_infinite_search(true);
		}

		if (parse_error)
		{
			_logger.write(_myname,
				"[%s] unable to set parameter '%s'\n",
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
		_logger.write(_myname,
			"[%s] unable to set searchmoves to '%s'\n",
			__FUNCTION__, moves.c_str());
		return false;
	}

	/*
	 * Request a state transition to StateMachine::init_search
	 */
	AbortIfNot(
			state_update_sig.is_connected(),
		false);

	AbortIfNot( state_update_sig.raise(this,
			StateMachine::init_search),
		false);

	return true;
}

/**
 * Initialize this interface
 *
 * @param[in] fd The file descriptor from which to read user commands
 *
 * @return True on success
 */
bool UCI::init(int fd, const Search* search)
{
	AbortIfNot(_cmd.init(fd), false);

	AbortIfNot(_init_outputs(search), false);
	AbortIfNot(_init_commands(), false);
	AbortIfNot(_init_options(),  false);

	AbortIfNot(_logger.register_source(_myname),
		false);

	_is_init = true;

	/*
	 * Send the "uci" command to the command interface in case it was
	 * sent by the GUI
	 */
	AbortIfNot( _cmd.handle_command("uci\n", 4),
		false);

	return true;
}

/**
 * The handler for the "isready" UCI command
 *
 * @return True on success
 */
bool UCI::isready(const std::string&) const
{
	AbortIfNot(write("readyok\n"),
		false);

	return true;
}

/**
 * The handler for the "ponderhit" UCI command
 *
 * @return True on success
 */
bool UCI::ponderhit(const std::string&) const
{
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
		_logger.write(_myname, "no arguments passed to [%s]\n",
			__FUNCTION__);
		return false;
	}

	Util::str_v args, parts;
	Util::split(_args, parts, "moves");

	Util::split(parts[0], args);

	if (args[0] != "startpos" && args[0] != "fen")
	{
		_logger.write(_myname,
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
			_logger.write(_myname,"missing FEN string\n");
			return false;
		}

		args.erase(args.begin());

		std::string fen = Util::build_string(args, " ");

		if (!pos.reset(fen, true))
		{
			_logger.write(_myname, "invalid FEN \"%s\"\n",
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

	MoveGen movegen(_tables);

	for (size_t i = 0; i < moves.size(); i++)
	{
		const int partial =
			Util::parse_coordinate(moves[i]);

		if (partial == 0)
		{
			_logger.write(_myname, "invalid move '%s'",
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
			_logger.write(_myname, "invalid move '%s'",
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

		/*
		 * Confirm that this move is legal:
		 */
		bool legal = false;

		Buffer<int,MAX_MOVES> buf;
		const int n_moves =
			movegen.generate_legal_moves(pos, pos.get_turn(),
				buf);
		for (int i = 0; i < n_moves; i++)
		{
			if (buf[i] == move)
			{
				legal = true; break;
			}
		}

		if (!legal)
		{
			_logger.write( _myname, "illegal move => '%s'\n",
				moves[i].c_str());
			return false;
		}

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
bool UCI::postsearch(EngineOutputs* outputs)
{
	outputs->update();

	int bestmove, ponder;
	AbortIfNot(outputs->get<int>(_bestmove_token, bestmove), false);

	AbortIfNot(outputs->get<int>(_ponder_token, ponder), false);

	const std::string bestmove_s =  Util::printCoordinate(bestmove);
	const std::string ponder_s   =
		Util::printCoordinate( ponder );

	std::string out = "bestmove " + bestmove_s;

	if (inputs.get_ponder())
		out += " ponder " + ponder_s;

	write("%s\n", out.c_str());

	AbortIfNot(
			state_update_sig.is_connected(),
		false);
	AbortIfNot( state_update_sig.raise(this,
			StateMachine::idle),
		false);

	return true;
}

/**
 * The handler for the "quit" command
 *
 * @return True on success
 */
bool UCI::quit(const std::string&)
{
	AbortIfNot(state_update_sig.is_connected(),
		false);

	AbortIfNot(state_update_sig.raise(this, StateMachine::exiting),
		false);

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
bool UCI::send_periodics(EngineOutputs& outputs) const
{
	outputs.update();

	std::string output = "info";

	std::string depth_s,
				nodes_s,
				time_s,
				score_s,
				mate_in_s,
				node_type_s,
				current_move_s,
				move_number_s,
				hash_permill_s,
				nps_s,
				pv;

	int depth;
	int64 nodes, time;
	int score, mate_in;
	bool fail_lo, fail_hi;
	int current_move, move_number;
	double hash_permill;
	int64 nps;

	AbortIfNot(outputs.get(_search_depth_token, depth), false);
	AbortIfNot(outputs.get(_nodes_searched_token, nodes), false);
	AbortIfNot(outputs.get(_search_time_token, time), false);
	time /= 1000000; // milliseconds

	AbortIfNot(outputs.get(_search_score_token, score), false);
	AbortIfNot(outputs.get(_mate_in_token, mate_in), false);
	AbortIfNot(outputs.get(_fail_hi_token, fail_hi), false);
	AbortIfNot(outputs.get(_fail_lo_token, fail_lo), false);
	AbortIfNot(outputs.get(_current_move_token, current_move), false);
	AbortIfNot(outputs.get(_current_movenumber_token, move_number), false);
	AbortIfNot(outputs.get(_hash_usage_token, hash_permill), false);
	hash_permill *= 10; // permill

	AbortIfNot(outputs.get(_nps_token, nps), false);
	AbortIfNot(outputs.get(_pv_token, pv), false);

	AbortIfNot(Util::to_string(depth, depth_s),
		false);
	AbortIfNot(Util::to_string(nodes, nodes_s),
		false);
	AbortIfNot(Util::to_string(time, time_s),
		false);
	AbortIfNot(Util::to_string(score, score_s),
		false);
	AbortIfNot(Util::to_string(mate_in, mate_in_s),
		false);

	if (fail_lo)
		node_type_s = "upperbound";
	if (fail_hi)
		node_type_s = "lowerbound";

	current_move_s = Util::printCoordinate(current_move);

	AbortIfNot(Util::to_string(move_number, move_number_s), false);
	AbortIfNot(Util::to_string((int)hash_permill, hash_permill_s), false);
	AbortIfNot(Util::to_string(nps, nps_s), false);

	output += " depth " + depth_s;
	output += " time "  + time_s;
	output += " nodes " + nodes_s;
	output += " pv "    + pv;
	
	/* pv or multipv */

	output += " score ";

	if (0 <= mate_in)
		output += "mate " + mate_in_s;
	else
		output += "cp "   + score_s;

	if (fail_hi)
		output += " lowerbound";
	else if (fail_lo)
		output += " upperbound";

	output += " currmove " + current_move_s;
	output += " currmovenumber " + move_number_s;
	output += " hashfull " + hash_permill_s;
	output += " nps " + nps_s;

	write("%s\n", output.c_str());
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
		_logger.write(_myname, "too few inputs '%s'\n",  _args.c_str() );

		return false;
	}

	auto iter = find_option(Util::trim(args[1]));

	if (iter == _options.end())
	{
		_logger.write(_myname, "unknown option '%s'\n", args[1].c_str());

		return false;
	}

	const option_base* option = *iter;

	if (option->type == "button")
	{
		auto button = dynamic_cast<const Button*>(option);
		return button->push();
	}

	/*
	 * If we got here, there should also be a value specified
	 * for this option:
	 */
	if (args.size() != 4
		|| Util::to_lower(Util::trim(args[0])) != "name"
		|| Util::to_lower(Util::trim(args[2])) != "value")
	{
		_logger.write(_myname, "invalid command syntax '%s'\n",
					  _args.c_str());
		return false;
	}

	if (!option->update(args[3]))
	{
		_logger.write(
				_myname, "failed to set option '%s' to '%s'\n",
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
 * Handles the UCI "stop" command
 *
 * @return True on success
 */
bool UCI::stop(const std::string&)
{
	AbortIfNot(
			state_update_sig.is_connected(),
		false);

	AbortIfNot( state_update_sig.raise(this,
			StateMachine::post_search),
		false);

	return true;
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

	AbortIfNot(write("id name %s\n" , name.c_str()),
		false);
	AbortIfNot(write("id author Jason Fernandez\n"),
		false);

	for (auto iter=_options.begin(), end= _options.end(); iter != end;
		 ++iter)
	{
		const auto& ptr = *iter;

		std::string output = "option name " + ptr->name
			+ " type " + ptr->type;

		if (ptr->type == "check")
		{
			auto opt = dynamic_cast<const Check*>(ptr);
			std::string str;
			AbortIfNot(Util::to_string<bool>(opt->default_value, str),
				false);
			output += " default " + str;
		}
		else if (ptr->type == "combo")
		{
			auto opt = dynamic_cast<const Combo*>(ptr);
			output +=
				" default " + opt->default_value;

			for (size_t i=0; i < opt->vars.size(); i++)
			{
				output += " var " + opt->vars[i];
			}
		}
		else if (ptr->type == "spin")
		{
			auto opt = dynamic_cast<const Spin*>(ptr);
			std::string str;
			AbortIfNot(Util::to_string<int >(opt->default_value, str),
				false);

			output += " default " + str;

			AbortIfNot(Util::to_string<int >(opt->min, str),
				false);
			output += " min " + str;

			AbortIfNot(Util::to_string<int >(opt->max, str),
				false);
			output += " max " + str;
		}
		else if (ptr->type == "string")
		{
			auto opt = dynamic_cast<const String*>(ptr);
			output +=
				" default " + opt->default_value;
		}

		AbortIfNot(write("%s\n",
			output.c_str()), false);
	}

	/*
	 * Send the "uciok" footer:
	 */
	AbortIfNot(write("uciok\n"),
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
	AbortIfNot(
			state_update_sig.is_connected(),
		false);

	AbortIfNot( state_update_sig.raise(this,
			StateMachine::idle),
		false);

	return true;
}

xBoard::xBoard(const DataTables& tables, EngineInputs& inputs,
			   Logger& logger)
	: Protocol("xBoard", tables, inputs, logger),
	  _is_init(false),
	  _logger(logger)
{
}

xBoard::~xBoard()
{
}

bool xBoard::init(int fd, const Search* search)
{
	AbortIfNot(_cmd.init(fd),
		false);

	AbortIfNot(_logger.register_source(_myname),
		false);

	_is_init = true;
	return true;
}

bool xBoard::postsearch(EngineOutputs* outputs)
{
	return true;
}

bool xBoard::send_periodics(EngineOutputs& outputs) const
{
	return true;
}

bool xBoard::sniff()
{
	return true;
}

Console::Console(const DataTables& tables, EngineInputs& inputs,
				 Logger& logger)
	: Protocol("Console", tables, inputs, logger),
	  _is_init(false),
	  _logger(logger)
{
}

Console::~Console()
{
}

bool Console::init(int fd, const Search* search)
{
	AbortIfNot(_cmd.init(fd),
		false);

	AbortIfNot(_logger.register_source(_myname),
		false);

	_is_init = true;
	return true;
}

bool Console::postsearch(EngineOutputs* outputs)
{
	return true;
}

bool Console::send_periodics(EngineOutputs& outputs) const
{
	return true;
}

bool Console::sniff()
{
	return true;
}
