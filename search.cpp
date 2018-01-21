#include "search2.h"

/**
 * Constructor
 *
 * @param[in] name    The name of this software component
 * @param[in] movegen A move generator
 */
Search::Search(const std::string& name,
			   const MoveGen& movegen)
	: StateMachineClient(name),
	  _movegen(movegen),
	  _outputs()
{
}

/**
 * Initialize. This can be overridden for a specific
 * search algorithm
 *
 * @return True on success
 */
bool Search::init()
{
	return true;
}

/**
 * Destructor
 */
Search::~Search()
{
}

EngineOutputs* Search::get_outputs()
{
	return &_outputs;
}

const EngineOutputs& Search::get_outputs() const
{
	return _outputs;
}

/**
 * Constructor
 *
 * @param[in] movegen A MoveGen, which generates moves at
 *                    each tree node
 * @param[in] sm      The chess engine state machine
 * @param[in] logger  The logger for writing diagnostics
 * @param[in] tables  The global pre-computed tables
 */
PvSearch::PvSearch(const MoveGen& movegen,
				   StateMachine& sm,
				   Logger& logger,
				   const DataTables& tables)
	: Search("PvSearch", movegen),
	  _abort_requested(false),
	  _best_move(0),
	  _depth(0),
	  _input_check_delay(100000),
	  _interrupt_handler(sm),
	  _is_init(false),
	  _logger(logger),
	  _mate_search(false),
	  _max_depth(1),
	  _next_input_check(0),
	  _node_count(0),
	  _node_limit(0),
	  _nps(0),
	  _ponder_move(0),
	  _qnode_count(0),
	  _search_score(0),
	  _start_time(0),
	  _stop_time (0),
	  _tables(tables)
{
}

/**
 * Destructor
 */
PvSearch::~PvSearch()
{
}

/**
 * Get the best move. This is updated after each search iteration
 *
 * @return The best move
 */
int PvSearch::get_best_move() const
{
	return _best_move;
}

/**
 * Get the move the engine will ponder on if pondering is enabled
 *
 * @return The move to ponder
 */
int PvSearch::get_ponder_move() const
{
	return _ponder_move;
}

/**
 * Retrieve the principal variation from the most recent search
 *
 * @param [in] pos A position from which to play this variation
 *
 * @return The principal variation
 */
std::string PvSearch::get_pv(Position& pos) const
{
	int move_number = pos.get_fullmove_number();
	int to_move = pos.get_turn();

	BUFFER( int, moves, MAX_MOVES );
	std::string pv = "";

	for (int ply = 0; _pv[0][ply]; ply++)
	{
		const int pv_move = _pv[0][ply];

		const int n_moves = 
			_movegen.generate_legal_moves( pos, to_move, moves );

		int match = -1;

		for (int i = 0; i < n_moves; i++)
		{
			if (TO(moves[i] == TO(pv_move))
				&& FROM(moves[i]) == FROM(pv_move))
			{
				match = moves[i]; break;
			}
		}

		if (match == -1)
		{
			_logger.write( _name, "invalid PV move: %s\n",
				Util::printCoordinate(pv_move));
			return "";
		}

		if (ply == 0 || to_move == WHITE)
		{
			std::string str;
			AbortIfNot(Util::to_string<int>(move_number++, str),
				"");
			pv += str + ". ";
		}

		if (to_move == BLACK && ply == 0)
			pv += " ... ";

		std::string move;

		pos.make_move(match); to_move = pos.get_turn();
		const bool in_check= pos.in_check(to_move);

		if (in_check)
		{
			const int n_moves = 
				_movegen.generate_check_evasions(pos,
					to_move, moves);

			if (n_moves > 0)
				move = Util::format_move(match, in_check);
			else
			{
				move = Util::format_move(match);
				move += "#";
			}
		}

		pv += move + " ";
	}

	return pv;
}

/**
 * Initialized this object. This must be done prior to attempting
 * a \ref search()
 *
 * @return True on success
 */
bool PvSearch::init()
{
	AbortIfNot(_logger.register_source(_name),
		false);

	/*
	 * Create the outputs we'll send the GUI
	 */
	AbortIf(_outputs.create(
		"ponder", *this, &PvSearch::get_ponder_move) < 0, false);

	AbortIf(_outputs.create(
		"bestmove", *this, &PvSearch::get_best_move) < 0, false);

	_is_init = true;
	return true;
}

/**
 * Check if \a to_move was forced into checkmate during the last
 * search iteration
 *
 * @param[in] to_move See if this side is mated
 *
 * @return True if \a to_move is forced into checkmate
 */
bool PvSearch::is_mated(int to_move) const
{
	if (to_move == WHITE)
		return _search_score == -MATE_SCORE;
	else
		return _search_score ==  MATE_SCORE;
}

/**
 * Run a new search
 *
 * @param [in] inputs Configure the search with these parameters
 *
 * @return True on success
 */
bool PvSearch::search(const EngineInputs* inputs)
{
	AbortIfNot(_is_init, false);

	AbortIfNot(
			state_update_sig.is_connected(),
		false);

	AbortIfNot( state_update_sig.raise(this,
			StateMachine::searching),
		false);

	_start_time = Clock::get_monotonic_time();

	set_inputs(*inputs);

	_abort_requested  = false;
	_next_input_check = _input_check_delay;

	_node_count  = 0;
	_qnode_count = 0;

	const Position& master = *inputs->get_position();

	const int to_move   = master.get_turn();
	const bool in_check =
			master.in_check(to_move);

	const int sign = (to_move == WHITE ? 1 : -1);

	if (to_move == WHITE)
		_search_score = -MATE_SCORE;
	else
		_search_score =  MATE_SCORE;

	int moves[MAX_MOVES];

	_best_move = _ponder_move = 0;

	Position pos(master);

	size_t n_moves;
	if (in_check)
	{
		n_moves =
			_movegen.generate_check_evasions(pos, to_move, moves);
	}
	else
	{
		n_moves =
			_movegen.generate_legal_moves(pos, to_move, moves);
	}

	Util::bubble_sort(moves, n_moves);

	for (_depth = 0; _depth < _max_depth; _depth++)
	{
		int alpha = -MATE_SCORE;
		int beta  =  MATE_SCORE;

		int best_move = 0;
		const int score = -sign * 
			_search_moves(pos, moves, n_moves, alpha, beta, 0,
				!in_check, best_move);

		if (_depth > 0 && _abort_requested)
		{
			break;
		}
		else
		{
			if ((to_move == WHITE && score > _search_score)
				|| (to_move == BLACK && score < _search_score))
			{
				_best_move    = best_move;
				_search_score = score;
					save_pv(0, best_move);
			}
		}

		if (is_mated(to_move))
			break;
	}

	if (!_best_move)
		_best_move = moves[0];

	/*
	 * Set the move to ponder on:
	 */
	_ponder_move = _pv[0][1];

	/*
	 * Transition into the post_search state:
	 */
	AbortIfNot(state_update_sig.raise(
			this, StateMachine::post_search),
		false);

	return true;
}

/**
 * Set the search inputs
 *
 * @oaram[in] inputs "go" input parameters
 */
void PvSearch::set_inputs(const EngineInputs& inputs)
{
	const int to_move = inputs.get_position()->get_turn();

	const int total_time = inputs.get_time( to_move );

	/*
	 *  1. Limit the search time (milliseconds). We always
	 *     give ourselves at least 1 second
	 */
	int time = total_time;

	if (inputs.use_fixed_searchtime())
	{
		time = inputs.get_movetime();
	}
	else
	{
		const int moves_left = inputs.get_movestogo();
		if (moves_left != 0)
		{
			time = _max(1000,total_time / moves_left);
		}
	}

	_stop_time = Clock::get_monotonic_time() +
		(time * MILLION);

	/*
	 *  2. Limit the search depth (plies)
	 */
	if (inputs.get_mate_search())
	{
		_mate_search = true;
		_max_depth = _max(0, inputs.get_mate_depth() * 2 - 1);
	}
	else
	{
		_max_depth =  inputs.use_fixed_searchdepth() ?
			inputs.get_depth() : MAX_PLY/2;
	}

	/*
	 * 3. Limit the number of nodes to search
	 */
	_node_limit = inputs.use_fixed_searchnodes() ?
		inputs.get_node_limit() : MAX_INT64;


	_logger.write(_name, "search time  = %d ms\n",
		time);
	_logger.write(_name, "search depth = %d plies\n",
		_max_depth);
	_logger.write(_name, "node limit   = %lld\n",
		_node_limit);
}

/**
 * Clear the principal variation
 */
void PvSearch::_clear_pv()
{
	for (register int i = 0; i < MAX_PLY; i++)
		for (register int j = 0; j < MAX_PLY; j++)
			_pv[i][j] = 0;
}
