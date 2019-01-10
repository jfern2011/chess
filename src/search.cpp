#include <limits>

#include "search.h"

namespace Chess
{
	/**
	 * Constructor
	 */
	Search::Search() : _is_init(false), _position()
	{
		_set_defaults();
	}

	/**
	 * Destructor
	 */
	Search::~Search()
	{
	}

	/**
	 * Grab the principal variation from the most recent
	 * depth iteration
	 *
	 * @note The PV ends when a null move is encountered
	 *
	 * @return The principal variation
	 */
	MoveList Search::get_pv() const
	{
		MoveList list; list.init(_pv[0], 0);

		for (size_t i = 0;
			 _pv[0][i] && i < max_ply; i++ )
			list.size++;

		return list;
	}

	/**
	 *  Initialize for a new search. This must be called
	 *  prior to every \ref run()
	 *
	 * @param[in] pos The position to search
	 *
	 * @return True on success
	 */
	bool Search::init(Handle<Position> pos)
	{
		if (!pos) return false;

		_position = pos;

		_set_defaults();

		_is_init = true;
		return true;
	}

	/**
	 * Quiescent search routine. This searches captures,
	 * promotions, and checks only
	 *
	 * @param[in] depth The current depth
	 * @param[in] alpha The search window lower bound
	 * @param[in] beta  The search window upper bound
	 *
	 * @return The score score (relative to us)
	 */
	int16 Search::quiesce(int depth, int16 alpha, int16 beta)
	{
		Position& pos = *_position;

		const player_t to_move = pos.get_turn();
		const bool in_check = pos.in_check(to_move);

		const auto & tables = DataTables::get();

		BUFFER(int32, moves, max_moves);
		size_t n_moves;

		if (in_check)
		{
			n_moves = MoveGen::generate_check_evasions(
				pos, moves);

			if (n_moves == 0)
			{
				/*
				 * Mark the end of this variation:
				 */
				save_pv(depth, 0);

				/*
			 	 * Add a small penalty to the mate score to encourage
			 	 * mates closer to the root:
			 	 */
				return depth - king_value;
			}
		}

		/*
		 * Get an initial score for this position:
		 */
		const int16 score =
			tables.sign[pos.get_turn()] * evaluate(pos);

		/*
		 * Check if we can fail-high; not sure if this is correct for
		 * zugzwang positions...
		 */
		if (score >= beta)
			return beta;

		if (alpha < score) alpha = score;

		if (!in_check)
			n_moves = MoveGen::generate_captures(
				pos, moves);

		/*
		 * Return the heuristic value of this position if
		 * no captures are left:
		 */
		if (n_moves == 0 || max_ply <= depth)
		{
			save_pv(depth, 0);
			return score;
		}

		SelectionSort sort;
			sort.init(moves, n_moves);

		int best_move = 0;
		int32 move;

		while (sort.next(move, [](int32 mv1, int32 mv2) {
				return Chess::score( mv1 ) - Chess::score( mv2 );
			}))
		{
			if (!in_check)
			{
				/*
				 * If this is a promotion, make the move and run a
				 * see() on the "to" square. If we can't promote the
				 * pawn without it getting captured, don't bother
				 * searching this move
				 */
				if (extract_captured(move) == piece_t::empty &&
					extract_promote (move) != piece_t::empty)
				{
					pos.make_move  (move);
					const int score = see(pos, pos.get_turn(),
										  extract_to(move));
					pos.unmake_move(move);

					if ( score > 0 )
						continue;
				}

				/*
				 * Perform a see() on captures that might be losing,
				 * e.g. QxP. If a see() value is negative, don't
				 * bother searching the capture since chances are it
				 * won't help our position
				 */
				else if (tables.piece_value[extract_captured(move)]
					< tables.piece_value[extract_moved(move)])
				{
					if (see(pos, pos.get_turn(), extract_to(move))
						< 0) continue;
				}
			}

			pos.make_move(move);

			_node_count++; _qnode_count++;

			const int score =
				-quiesce( depth+1, -beta, -alpha );

			pos.unmake_move(move);

			if (score >= beta) return beta;

			if ( score > alpha )
			{
				best_move = move;
				alpha = score;
			}
		}

		save_pv(depth, best_move);

		return alpha;
	}

	/**
	 * Run the search algorithm
	 *
	 * @param[in]  timeout The maximum number of milliseconds
	 *                     to run the search for
	 * @param[in]  depth   The maximum iteration depth
	 * @param[out] best    The best move
	 *
	 * @return The search score (relative to us)
	 */
	int16 Search::run(int timeout, int depth, int32 best)
	{
		if (!_is_init)
		{
			return std::numeric_limits<int16>::max();
		}

		_set_defaults(); _is_init = false;

		lines.resize(20); // Number of lines to save

		return search_root();
	}

	/**
	 * Back up the principal variation from the given depth
	 *
	 * @param [in] depth The starting depth
	 * @param [in] move  The move to save at depth \a depth
	 */
	void Search::save_pv(int depth, int move)
	{
		if (depth < max_ply)
		{
			_pv[depth][depth] = move;

			// Null move signals the end of a variation:
			if (move == 0) return;
		}

		for (register int i = depth+1; i < max_ply; i++)
		{
			if ((_pv[depth][i] = _pv[depth+1][i])
				== 0) break;
		}
	}

	/**
	 * Implements the recursive negamax search algorithm
	 *
	 * @param[in] depth The current depth
	 * @param[in] alpha The search window lower bound
	 * @param[in] beta  The search window upper bound
	 *
	 * @return The search score (relative to us)
	 */
	int16 Search::search(int depth, int16 alpha, int16 beta)
	{
		Position& pos = *_position;

		const bool in_check = pos.in_check(pos.get_turn());

		/*
		 * Don't quiece() if we're in check:
		 */
		if (_iteration_depth <= depth && !in_check)
			return quiesce(depth, alpha, beta);

		const int16 init_alpha = alpha;
		int best_move = 0;

		SearchPhase phase;

		if (in_check)
		{
			phase.init<phase_t::check_evasions>(pos);

			if (phase.evasions.size == 0)
			{
				/*
				 * Mark the end of this variation:
				 */
				save_pv(depth, 0);

				/*
			 	 * Add a small penalty to the mate score to encourage
			 	 * mates closer to the root:
			 	 */
				return depth - king_value;
			}

			const int16 score = 
				search_moves<phase_t::check_evasions>(phase, alpha,
					beta, depth, best_move);

			if ( beta <= score )
				return beta;

			if (alpha > init_alpha)
				save_pv(depth, best_move);

			return alpha;
		}

		/*
		 * 1. Search winning captures
		 */

		phase.init<phase_t::winning_captures>(pos);

		int16 score = search_moves< phase_t::winning_captures >(
			phase, alpha, beta, depth, best_move);

		if ( beta <= score )
			return beta;

		/*
		 * 2. Search non-captures
		 */

		phase.init<phase_t::non_captures>(pos);

		score = search_moves< phase_t::non_captures >(
			phase, alpha, beta, depth, best_move);

		if ( beta <= score )
			return beta;

		/*
		 * 3. If no captures/non-captures are available,
		 *    then it is a draw
		 */
		if (phase.winning_captures.size == 0 &&
			phase.non_captures.size == 0)
		{
			return 0;
		}

		/*
		 * 4. Search losing captures
		 */

		phase.init<phase_t::losing_captures>(pos);

		score = search_moves< phase_t::losing_captures >(
			phase, alpha, beta, depth, best_move);

		if ( beta <= score )
			return beta;

		if (alpha > init_alpha)
			save_pv(depth, best_move);

		return alpha;
	}

	/**
	 * Similar to \ref search(), but for the root node only
	 *
	 * @return The search score (relative to us)
	 */
	int16 Search::search_root()
	{
		Position& pos = *_position;

		const bool in_check =
			pos.in_check(pos.get_turn());

		BUFFER(int32, moves, max_moves );
		size_t n_moves;

		if (in_check)
		{
			n_moves = MoveGen::generate_check_evasions(
				pos, moves);

			if (n_moves == 0)
			{
				save_pv(0, 0); return -king_value;
			}
		}
		else
		{
			n_moves = MoveGen::generate_captures(
				pos, moves);

			n_moves += MoveGen::generate_noncaptures(
				pos, &moves[n_moves]);
		}

		if (n_moves == 0) return 0;

		auto best = std::make_pair<int32,int16>(
						0, king_value+1);

		for (size_t i = 0; i < n_moves; i++)
		{
			const int32 move = moves[i];

			pos.make_move( move );

			const int16 score = search(1, -king_value,
				king_value);

			pos.unmake_move(move);

			save_pv(0, move);

			if (score < best.second)
			{
				best.first = move; best.second
					= score;
			}

			lines.insert(get_pv(), -score);
		}

		return -best.second;
	}

	/**
	 * Set default search values. Used during initialization
	 * and construction
	 */
	void Search::_set_defaults()
	{
		_iteration_depth = 3;
			_node_count = _qnode_count = 0;

		for (size_t i= 0; i < max_ply; i++)
			_pv[0][i] = 0;

		lines.clear();
	}
}
