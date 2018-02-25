#include "protocol2.h"
#include "search2.h"

// Test position 1: 2q3k1/3p4/4p3/8/4R1B1/8/5P2/4Q1K1 w - - 0 1
// Test position 2: 4q1k1/5p2/8/4r1b1/8/4P3/5P2/2Q3K1 b - - 0 1
// Test position 3: r6k/6pp/7N/8/8/1Q6/8/7K w - - 0 1
// Test position 4: 7k/8/1q6/8/8/7n/6PP/R6K b - - 0 1

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
 * Send periodic output to the GUI while a search is running
 *
 * @return True on success
 */
bool Search::send_periodics()
{
	return true;
}

/**
 * Constructor
 *
 * @param[in] movegen  A MoveGen, which generates moves at
 *                     each tree node
 * @param[in] sm       The chess engine state machine
 * @param[in] protocol The GUI interface
 * @param[in] logger   The logger for writing diagnostics
 * @param[in] tables   The global pre-computed tables
 */
PvSearch::PvSearch(const MoveGen& movegen,
				   StateMachine& sm,
				   Logger& logger,
				   const Protocol* protocol,
				   const DataTables& tables)
	: Search("PvSearch", movegen),
	  _abort_requested(false),
	  _best_move(0),
	  _depth(0),
	  _fail_high(false),
	  _fail_low(false),
	  _infinite(false),
	  _input_check_delay(100000),
	  _interrupt_handler(sm),
	  _is_init(false),
	  _logger(logger),
	  _mate_search(false),
	  _max_depth(1),
	  _movenum(1),
	  _next_input_check(0),
	  _node_count(0),
	  _node_limit(0),
	  _nps(0),
	  _num_pv(1),
	  _ponder_move(0),
	  _protocol(protocol),
	  _pv_stack(),
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
 * Get the current depth being searched to
 *
 * @return The search depth
 */
int PvSearch::current_depth() const
{
	return _depth;
}

/**
 * Get the 21-bit representation of the move currently being searched
 *
 * @return The move being searched
 */
int PvSearch::current_move() const
{
	return _current_move[0];
}

/**
 * Get current move number of the move being searched
 *
 * @return The move number
 */
int PvSearch::current_move_number() const
{
	return _movenum;
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
 * Get the current best lines from a search in progress. Each
 * line is separated by a '\n'
 *
 * @note Assumes the lines are sorted
 *
 * @return The best lines
 */
std::string PvSearch::get_lines() const
{
#if 0
	Util::str_v lines;
	for (auto iter = _pv_stack.begin(), end = _pv_stack.end();
		 iter != end; ++iter)
		lines.push_back(iter->first);

	return Util::build_string(lines, "\n");
#endif
	return _pv_stack.front().first;
}

/**
 * Get the number of best lines requested by the user
 *
 * @return The number of lines
 */
int PvSearch::get_num_lines() const
{
	return _num_pv;
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
#ifdef CONSOLE_MODE
	int move_number = pos.get_fullmove_number();
	int to_move = pos.get_turn();

	BUFFER( int, moves, MAX_MOVES );
	std::string pv = "";

	for (int ply = 0; _pv[0][ply]; ply++)
	{
		const int pv_move = _pv[0][ply];

		const int n_moves = 
			_movegen.generate_legal_moves( pos, to_move, moves );

		int match_ind, match = -1;

		for (int i = 0; i < n_moves; i++)
		{
			if (TO(moves[i]) == TO(pv_move)
				&& FROM(moves[i]) == FROM(pv_move))
			{
				match = moves[i]; match_ind = i;
				break;
			}
		}

		if (match == -1)
		{
			_logger.write( _name, "invalid PV move: %s\n",
				Util::printCoordinate(pv_move));
			return "";
		}

		/*
		 * If two pieces can move to the same square, specify
		 * the rank or file:
		 */
		std::string file_or_rank;
		if (MOVED(match) != PAWN && MOVED(match) != KING)
		{
			for (int i = 0; i < n_moves; i++)
			{
				if (i == match_ind) continue;
				if (MOVED(match) == MOVED(moves[i]) &&
					TO(match) == TO(moves[i]))
				{
					if (RANK(FROM(match)) == RANK(FROM(moves[i])))
						file_or_rank = Util::to_file(FILE(match));

					if (FILE(FROM(match)) == FILE(FROM(moves[i])))
					{
						Util::to_string(RANK(match)+1,
							file_or_rank);
					}
				}
			}
		}

		if (ply == 0 || to_move == WHITE)
		{
			std::string str;
			AbortIfNot(Util::to_string<int>(move_number++, str),
				"");
			pv += str + ". ";
		}

		if (to_move == BLACK && ply == 0)
			pv += "... ";

		pos.make_move(match); to_move = pos.get_turn();
		const bool in_check= pos.in_check(to_move);

		std::string move = Util::format_move( match, file_or_rank );

		if (in_check)
		{
			const int n_moves = 
				_movegen.generate_check_evasions(pos, to_move,
					moves);

			if (n_moves > 0)
			{
				move = Util::format_move(match, file_or_rank, true);
			}
			else
			{
				move += "#";
			}
		}

		pv += move + " ";
	}
#else
	std::string pv = "";
	for (int ply = 0; _pv[0][ply]; ply++)
	{
		auto move = Util::printCoordinate(_pv[0][ply]);
		pv += move + " ";
	}
#endif

	return pv;
}

/**
 * Get the current search rate in nodes per second
 *
 * @return The search rate
 */
int64 PvSearch::get_search_rate() const
{
	return _nps;
}

/**
 * Get the current (optimal) score produced by this search
 *
 * @return The score
 */
int PvSearch::get_search_score() const
{
	return _search_score;
}

/**
 * Get the percentage of the hash table being used
 *
 * @return The hash table usage
 */
double PvSearch::hash_usage() const
{
	return 0;
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

	AbortIf(_outputs.create(
		"pv", *this, &PvSearch::get_lines) < 0, false);

	AbortIf(_outputs.create(
		"search_depth", *this, &PvSearch::current_depth) < 0, false);

	AbortIf(_outputs.create(
		"nodes_searched", *this, &PvSearch::nodes_searched) < 0, false);

	AbortIf(_outputs.create(
		"search_time", *this, &PvSearch::time_used) < 0, false);

	AbortIf(_outputs.create(
		"nlines", *this, &PvSearch::get_num_lines) < 0, false);

	AbortIf(_outputs.create(
		"search_score", *this, &PvSearch::get_search_score) < 0, false);

	AbortIf(_outputs.create(
		"mate_in", *this, &PvSearch::mate_in) < 0, false);

	AbortIf(_outputs.create(
		"fail_hi", *this, &PvSearch::is_lower_bound) < 0, false);

	AbortIf(_outputs.create(
		"fail_lo", *this, &PvSearch::is_upper_bound) < 0, false);

	AbortIf(_outputs.create(
		"current_move", *this, &PvSearch::current_move) < 0, false);

	AbortIf(_outputs.create(
		"current_move_number", *this, &PvSearch::current_move_number) < 0, false);

	AbortIf(_outputs.create(
		"hash_usage", *this, &PvSearch::hash_usage) < 0, false);

	AbortIf(_outputs.create(
		"nps", *this, &PvSearch::get_search_rate) < 0, false);

	_is_init = true;
	return true;
}

/**
 * Push a new PV onto the PV stack unless the stack is full,
 * in which case the PV is only inserted if it is better than an
 * existing entry. This also sorts the stack
 *
 * @param[in] pv    The principal variation to add
 * @param[in] score The score associated with this PV
 */
void PvSearch::insert_pv(const std::string& pv, int score)
{
	_pv_stack.push_back(std::make_pair(pv, score));

	_pv_stack.sort([](const pv_score_p& a, const pv_score_p& b) {

					if (a.second == b.second)
						return a.first.size() > b.first.size();
					else
						return a.second > b.second;

					});

	if (_pv_stack.size() > MAX_PV)
		_pv_stack.pop_back();
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
 * Check if the last score returned was only a lower bound
 *
 * @return True if the score is a lower bound
 */
bool PvSearch::is_lower_bound() const
{
	return _fail_high;
}

/**
 * Check if the last score returned was only an upper bound
 *
 * @return True if the score is an upper bound
 */
bool PvSearch::is_upper_bound() const
{
	return _fail_low;
}

/**
 * If the search found a forced mate, get the number of moves
 * to checkmate
 *
 * @return Y, where Y is the number of moves to checkmate, or
 *         -1 if no mate was found
 */
int PvSearch::mate_in() const
{
	if (MATE_SCORE - _abs(_search_score) <= MAX_PLY)
	{
		int depth = MATE_SCORE - _abs(_search_score);
		if (depth % 2 == 0) return depth / 2;
		else return (depth+1)/2;
	}

	return -1;
}

/**
 * Get the number of nodes visited during this search
 *
 * @return The node count
 */
int64 PvSearch::nodes_searched() const
{
	return _node_count;
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

	int moves[MAX_MOVES];

	_best_move = _ponder_move = 0;

	/*
	 * Clear the PVs from the previous search
	 */
	_pv_stack.clear();

	size_t n_moves;
	if (in_check)
	{
		n_moves =
			_movegen.generate_check_evasions(master, to_move, moves);
	}
	else
	{
		n_moves =
			_movegen.generate_legal_moves(master, to_move, moves);
	}

	_best_move = moves[0];

	Util::bubble_sort(moves, n_moves);

	_search_score = 0;

	for (_depth = 0; _depth < _max_depth || _infinite; _depth++)
	{
		int temp_score = -MATE_SCORE * 2;

		int alpha = -MATE_SCORE;
		int beta  =  MATE_SCORE;

		Position pos = master;

		_clear_pv();

		int best_move = 0;
		const int score =
			_search_moves(pos, moves, n_moves, alpha, beta, 0,
				!in_check, best_move);

		if (_depth > 0 && _abort_requested)
			break;

 		if (score > temp_score)
		{
			_best_move    = best_move;
			temp_score = score;
				save_pv(0, best_move);
		}

		insert_pv(get_pv(pos), temp_score);

		_search_score = temp_score;

		send_periodics();
	}

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
 * Send periodic outputs to the GUI while a search
 * is running
 *
 * @return True on success
 */
bool PvSearch::send_periodics()
{
	AbortIfNot(_protocol->send_periodics(_outputs),
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

	_infinite = inputs.run_infinite_search();
	if (_infinite)
	{
		_logger.write( _name, "infinite search mode.\n" );
		return;
	}

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
			// TODO: What if we aren't told how many moves
			//       are left?

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

	_num_pv = inputs.get_multipv();
}

/**
 * Get the time elapsed since the start of the search
 *
 * @return The search time, in nanoseconds
 */
int64 PvSearch::time_used()
{
	return Clock::get_monotonic_time() - _start_time;
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
