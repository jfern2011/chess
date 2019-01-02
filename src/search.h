#ifndef __SEARCH_H__
#define __SEARCH_H__

#include <functional>

#include "eval.h"
#include "MoveGen4.h"
#include "see.h"
#include "SearchPhase.h"

namespace Chess
{
	/**
	 * Alpha-beta negamax search algorithm
	 */
	class Search final
	{

	public:

		Search();

		~Search();

		std::string get_pv() const;

		bool init(Handle<Position> pos);

		int16 quiesce(int depth, int16 alpha, int16 beta);

		int16 run(int timeout, int depth, int32 best);

		void save_pv(int depth, int move);

		int16 search(int depth, int16 alpha, int16 beta);

		template <phase_t P>
		int16 search_moves(SearchPhase& phase,
						   int16& alpha, int16 beta,
						   int depth, int& best);

	private:

		void _set_defaults();

		bool _is_init;

		int _iteration_depth;

		int64 _node_count;

		Handle< Position >
			_position;

		BUFFER(int, _pv, max_ply, max_ply);

		int64 _qnode_count;
	};

	/**
	 * Back up the principal variation from the given depth
	 *
	 * @param [in] depth The starting depth
	 * @param [in] move  The move to save at depth \a depth
	 */
	inline void Search::save_pv(int depth, int move)
	{
		if (depth < max_ply)
		{
			_pv[depth][depth] = move;

			// Null move signals the end of a variation:
			if (move == 0)
				return;
		}

		for (register int i= depth+1; i < max_ply; i++)
		{
			if ((_pv[depth][i] = _pv[depth+1][i]) == 0)
				break;
		}
	}

	inline int16 Search::search(int depth, int16 alpha, int16 beta)
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
		 * 3. Search losing captures
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


	template <phase_t P>
	int16 Search::search_moves(SearchPhase& phase,
							   int16& alpha, int16 beta,
							   int depth, int& best)
	{
		int32 move;
		while (phase.next_move<P>(move))
		{
			++_node_count;

			_position->make_move (move);

			const int16 score =
				-search( depth + 1, -beta, -alpha );

			_position->unmake_move(move);

			if (beta <= score)
				return beta;

			if (score > alpha)
			{
				alpha = score;
				best = move;
			}
		}

		return alpha;
	}

	inline int16 Search::quiesce(int depth, int16 alpha, int16 beta)
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
}

#endif
