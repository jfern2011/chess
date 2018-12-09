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
	class Search final
	{

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

		int16 search(int depth, int16 alpha, int16 beta);
/*
		int16 search_captures(SelectionSort2& sort,
							  int16& alpha, int16 beta,
							  int depth);
*/
		int16 search_moves(SelectionSort& sort,
						   int16& alpha, int16 beta,
						   int depth);

		int16 search_moves(int32* moves, size_t n_moves,
						   int16& alpha, int16 beta,
						   int depth);

	private:

		using compare_fn_t =
			std::function<int(int32,int32)>;

		bool _is_init;

		int _iteration_depth;

		int64 _node_count;

		Handle< Position >
			_position;

		int64 _qnode_count;
	};

	inline int16 Search::search(int depth, int16 alpha, int16 beta)
	{
		Position& pos = *_position;

		const bool in_check = pos.in_check(pos.get_turn());

		/*
		 * Don't quiece() if we're in check:
		 */
		if (_iteration_depth <= depth && !in_check)
			return quiesce(depth, alpha, beta);

		BUFFER(int32, moves, max_moves);
		size_t n_moves;

		if (in_check)
		{
			n_moves = MoveGen::generate_check_evasions(
				pos, moves);

			if (n_moves == 0)
			{
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
				search_moves( sort, alpha, beta, depth );

			if ( beta <= score )
				return beta;
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
				alpha = score;
		}

		/*
		 * 2. Search non-captures
		 */

		BUFFER(int32, quiet_moves, max_moves);

		size_t n_quiet = MoveGen::generate_noncaptures(
			pos, quiet_moves);

		if (n_moves == 0 && n_quiet == 0)
			return 0; // draw

		const int16 score = search_moves(
			quiet_moves, n_quiet, alpha, beta, depth );

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
					alpha = score;

			} while (sort.next(
				next_move));
		}

		return alpha;
	}
/*
	inline int16 Search::search_captures(CaptureSort& sort,
										 int16& alpha, int16 beta,
										 int depth)
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
				alpha = score;
		}

		return alpha;
	}
*/
	inline int16 Search::search_moves(SelectionSort& sort,
									  int16& alpha, int16 beta,
									  int depth)
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
				alpha = score;
		}

		return alpha;
	}

	inline int16 Search::search_moves(int32* moves, size_t n_moves,
									  int16& alpha, int16 beta,
									  int depth)
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
				alpha = score;
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
			return score;
		}

		SelectionSort sort(moves, n_moves);

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
				alpha = score;
		}

		return alpha;
	}
}

#endif
