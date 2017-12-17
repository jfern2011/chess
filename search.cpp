#include "search2.h"

Search::Search(const MoveGen& movegen)
	: _data(),
	  _movegen(movegen)
{
}

Search::~Search()
{
}

const Search::SearchData& Search::get_search_data() const
{
	return _data;
}

PvSearch::PvSearch(const MoveGen& movegen,
				   StateMachine& state_machine,
				   Logger& logger,
				   const DataTables& tables)
	: Search(movegen),
	  _abort_requested(false),
	  _best_move(0),
	  _depth(1),
	  _input_check_delay(100000),
	  _interrupt_handler(state_machine),
	  _is_init(false),
	  _logger(logger),
	  _name("PvSearch"),
	  _next_input_check(0),
	  _node_count(0),
	  _qnode_count(0),
	  _search_score(0),
	  _tables(tables)
{
}

PvSearch::~PvSearch()
{
}

bool PvSearch::init()
{
	AbortIfNot(_logger.register_source(_name),
		false);

	_is_init = true;
	return true;
}

bool PvSearch::is_mated(int to_move) const
{
	if (to_move == WHITE)
		return _search_score == -MATE_SCORE;
	else
		return _search_score ==  MATE_SCORE;
}

bool PvSearch::search(const Position& master)
{
	AbortIfNot(_is_init, false);

	_abort_requested  = false;
	_next_input_check = _input_check_delay;

	_node_count  = 0;
	_qnode_count = 0;

	const int to_move   = master.get_turn();
	const bool in_check =
			master.in_check(to_move);

	if (to_move == WHITE)
		_search_score = -MATE_SCORE;
	else
		_search_score =  MATE_SCORE;

	int moves[MAX_MOVES];

	for (int depth = 0; depth < _depth; _depth++)
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
			return true;
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

	return true;
}
