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
	  _data(),
	  _movegen(movegen)
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

const Search::SearchData& Search::get_search_data() const
{
	return _data;
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
	  _depth(1),
	  _input_check_delay(100000),
	  _interrupt_handler(sm),
	  _is_init(false),
	  _logger(logger),
	  _next_input_check(0),
	  _node_count(0),
	  _qnode_count(0),
	  _search_score(0),
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
 * Initialized this object. This must be done prior to attempting
 * a \ref search()
 *
 * @return True on success
 */
bool PvSearch::init()
{
	AbortIfNot(_logger.register_source(_name),
		false);

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
bool PvSearch::search(const EngineInputs& inputs)
{
	AbortIfNot(_is_init, false);

	AbortIfNot(transition_sig.is_connected(),
		false);

	AbortIfNot(transition_sig.raise(_name, StateMachine::searching,
		false), false);

	_abort_requested  = false;
	_next_input_check = _input_check_delay;

	_node_count  = 0;
	_qnode_count = 0;

	const Position& master = *inputs.get_position();

	const int to_move   = master.get_turn();
	const bool in_check =
			master.in_check(to_move);

	if (to_move == WHITE)
		_search_score = -MATE_SCORE;
	else
		_search_score =  MATE_SCORE;

	int moves[MAX_MOVES];

	for (int depth = 0; depth < _depth; depth++)
	{
		Position pos(master);

		int alpha = -MATE_SCORE;
		int beta  =  MATE_SCORE;

		const size_t n_moves =
			_movegen.generate_legal_moves(pos, to_move, moves);

		Util::bubble_sort(moves, n_moves);

		int best_move;
		const int score =
			-_search_moves(pos, moves, n_moves, alpha, beta,
				depth, !in_check, best_move);

		if (depth > 0 && _abort_requested)
		{
			break;
		}
		else
		{
			if ((to_move == WHITE && score > _search_score)
				|| (to_move == BLACK && score < _search_score))
			{
				_search_score = score;
				_best_move =
					best_move;
			}
		}

		if (is_mated(to_move))
			return true;
	}

	/*
	 * Request the state machine to transition to the
	 * idle state
	 */
	AbortIfNot(transition_sig.raise(
			_name, StateMachine::idle, false),
		false);

	return true;
}
