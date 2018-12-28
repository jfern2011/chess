#ifndef __SEARCH_H__
#define __SEARCH_H__

#include <functional>

#include "eval.h"
#include "MoveGen4.h"
#include "see.h"
#include "selection_sort.h"
#include "selection_sort2.h"

namespace Chess
{
	/**
	 * Alpha-beta negamax search algorithm
	 */
	class Search final
	{

		using compare_fn_t =
			std::function<int(int32,int32)>;

	public:

		Search();

		~Search();

		/**
		 * Score a move. This is done as follows:
		 *
		 * 1. The score is preliminarily computed as the difference
		 *    in value between the captured and moved pieces
		 * 2. If the move is a promotion, see if we can advance the
		 *    pawn safely. If so, add a bonus equal to the value
		 *    of the piece promoted to
		 *
		 * @param[in] pos  A Position, used to see() captures
		 * @param[in] move The move to score
		 *
		 * @return The score
		 */
		static int score(Position& pos, int32 move)
		{
			auto& tables = DataTables::get();
			int score = tables.exchange[extract_captured(move)][
				extract_moved(move)];

			const piece_t promote = extract_promote(move);
			if (promote != piece_t::empty)
			{
				pos.make_move  (move);

				if (see(pos, pos.get_turn(), extract_to(move)) <= 0)
					score += tables.piece_value[promote];

				pos.unmake_move(move);
			}

			return score;
		}

		bool init(Handle<Position> pos);

		int16 quiesce(int depth, int16 alpha, int16 beta);

		int16 run(int timeout, int depth, int32 best);

		void save_pv(int depth, int move);

		int16 search(int depth, int16 alpha, int16 beta);
/*
		int16 search_captures(SelectionSort2<compare_fn_t>& sort,
							  int16& alpha, int16 beta,
							  int depth);
*/
		int16 search_moves(SelectionSort& sort,
						   int16& alpha, int16 beta,
						   int depth, int& best);

		int16 search_moves(int32* moves, size_t n_moves,
						   int16& alpha, int16 beta,
						   int depth, int& best);

	private:

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

			/*
			 * Search all check evasions in MVV-LVA order
			 */

			SelectionSort sort(moves, n_moves);

			const int16 score =
				search_moves( sort, alpha, beta, depth, best_move );

			if ( beta <= score )
				return beta;

			if (alpha > init_alpha)
				save_pv(depth, best_move);

			return alpha;
		}
		else
		{
			n_moves = MoveGen::generate_captures(
				pos, moves);
		}

		SelectionSort2<compare_fn_t>
			sort(moves, n_moves, [&pos](int32 mv1, int32 mv2) {
				return score(pos,mv1) - score(pos,mv2);
		});

		/*
		 * 1. Search winning captures
		 */
		int32 next_move = 0;

		while (sort.next(next_move))
		{
			/*
			 * Exit after we hit the first losing capture
			 */
			if (score(pos, next_move) < 0)
				break;

			++_node_count;
			pos.make_move  (next_move);
			const int16 score =
				-search(depth + 1, -beta, -alpha);
			pos.unmake_move(next_move);

			if (beta <= score)
				return beta;

			if ( score > alpha )
			{
				best_move = next_move;
				alpha = score;
			}
		}

		/*
		 * 2. Search non-captures
		 */

		BUFFER(int32, quiet_moves, max_moves);

		size_t n_quiet = MoveGen::generate_noncaptures(
			pos, quiet_moves);

		if (n_moves == 0 && n_quiet == 0)
		{
			save_pv(depth, 0);
			return 0; // draw
		}

		const int16 score = search_moves(
			quiet_moves, n_quiet, alpha, beta, depth, best_move );

		if ( beta <= score )
			return beta;

		/*
		 * 3. Search losing (remaining) captures
		 */

		if (next_move && !sort.empty())
		{
			do
			{
				++_node_count;
				pos.make_move  (next_move);

				int16 score =
					-search(depth + 1, -beta, -alpha);

				pos.unmake_move(next_move);

				if ( beta <= score )
					return beta;

				if ( score > alpha )
				{
					best_move = next_move;
					alpha = score;
				}

			} while (sort.next(
				next_move));
		}

		if (alpha > init_alpha)
			save_pv(depth, best_move);
		return alpha;
	}
/*
	inline int16 Search::search_captures(SelectionSort2<compare_fn_t>& sort,
										 int16& alpha, int16 beta,
										 int depth)
	{
		Position& pos = *_position;

		for (size_t i = 0; i < sort.size(); ++i)
		{
			int32 move; sort.next(move);

			++_node_count;
			pos.make_move  (move);

			const int16 score =
				-search(depth+1, -beta, -alpha);

			pos.unmake_move(move);

			if (beta <= score)
				return beta;

			if ( score > alpha )
				alpha = score;
		}

		return alpha;
	}
*/
	inline int16 Search::search_moves(SelectionSort& sort,
									  int16& alpha, int16 beta,
									  int depth, int& best)
	{
		Position& pos = *_position;

		for (size_t i = 0; i < sort.size(); ++i)
		{
			const int32 move = sort.next();

			++_node_count;
			pos.make_move  (move);

			const int16 score =
				-search(depth+1, -beta, -alpha);

			pos.unmake_move(move);

			if (beta <= score)
				return beta;

			if ( score > alpha )
			{
				alpha = score;
				best = move;
			}
		}

		return alpha;
	}

	inline int16 Search::search_moves(int32* moves, size_t n_moves,
									  int16& alpha, int16 beta,
									  int depth, int& best)
	{
		Position& pos = *_position;

		for (size_t i = 0; i < n_moves; ++i)
		{
			const int32 move = moves[ i ];

			++_node_count;
			pos.make_move  (move);

			const int16 score =
				-search(depth+1, -beta, -alpha);

			pos.unmake_move(move);

			if (beta <= score)
				return beta;

			if ( score > alpha )
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
		const int16 score = evaluate(pos);

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

		SelectionSort sort(moves, n_moves);

		int best_move = 0;

		for (size_t i = 0; i < n_moves; ++i)
		{
			const int32 move = sort.next();

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
