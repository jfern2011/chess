#ifndef __SEE_H__
#define __SEE_H__

#include <algorithm>

#include "Position4.h"

namespace Chess
{
	/**
	 * Used to play back a sequence of captures
	 */
	struct see_record
	{
		/** Constructor */
		see_record() : moved(), captured()
		{
		}

		/** Clear this record */
		void clear()
		{
			captured.clear();
			moved.clear();
		}

		/**
		 * The piece that performed the capture
		 */
		std::vector<piece_t> moved;

		/**
		 * The piece that was captured
		 */
		std::vector<piece_t>
			captured;
	};

	/**
	 * Static exchange evaluation. This computes the outcome of a sequence
	 * of captures on \a square
	 *
	 * Note: This can also be used to determine if it is safe to move to
	 *       \a square
	 *
	 * @param [in] Position The position to evaluate
	 * @param [in] to_move  Who captures first
	 * @param [in] square   Square on which to perform the static exchange
	 *                      evaluation
	 *
	 * @return Optimal value of the capture sequence
	 */
	inline int see(const Position& pos, player_t to_move,
				   square_t square
#ifdef SEE_TEST
				   , see_record& record
#endif
				   )
	{
		BUFFER(int,scores,64); scores[0] = scores[1] = 0;

		int n_moves = 0;

		const auto& tables = DataTables::get();

		uint64 bishops_queens =
			pos.get_bitboard<piece_t::bishop>(player_t::white) |
			pos.get_bitboard<piece_t::queen >(player_t::white) |
			pos.get_bitboard<piece_t::bishop>(player_t::black) |
			pos.get_bitboard<piece_t::queen >(player_t::black);

		uint64 rooks_queens =
			pos.get_bitboard< piece_t::rook >(player_t::white) |
			pos.get_bitboard< piece_t::queen>(player_t::white) |
			pos.get_bitboard< piece_t::rook >(player_t::black) |
			pos.get_bitboard< piece_t::queen>(player_t::black);

		BUFFER(uint64, attackers, 2);

		attackers[player_t::white] =
			pos.attacks_to(square, player_t::white);
		attackers[player_t::black] =
			pos.attacks_to(square, player_t::black);

		piece_t last_capture
			= pos.piece_on( square );

		/*
	 	 * Bitmap of the occupied squares. We'll update this
	 	 * as captures are made
	 	 */
		uint64 occupied = pos.get_occupied(player_t::white) |
						  pos.get_occupied(player_t::black);

		while (attackers[to_move])
		{
			uint64 pieces;

			n_moves++;
			scores[ n_moves ] = tables.piece_value[ last_capture ]
				- scores[ n_moves-1 ];

#ifdef SEE_TEST
			record.captured.push_back(last_capture);
#endif

			/*
			 * Pruning: if the material gained still results
			 * in an overall loss, we can quit:
			 *
			if (std::max(-scores[n_moves-1],scores[n_moves])
					< 0) break;*/

			do {

				pieces = pos.get_bitboard< piece_t::pawn >(to_move)
							& attackers[to_move];

				if (pieces && last_capture != piece_t::empty)
				{
					last_capture = piece_t::pawn;
					break;
				}

				pieces = pos.get_bitboard<piece_t::knight>(to_move)
							& attackers[to_move];

				if (pieces)
				{
					last_capture = piece_t::knight;
					break;
				}

				pieces = pos.get_bitboard<piece_t::bishop>(to_move)
							& attackers[to_move];

				if (pieces)
				{
					last_capture = piece_t::bishop;
					break;
				}

				pieces = pos.get_bitboard< piece_t::rook >(to_move)
							& attackers[to_move];

				if (pieces)
				{
					last_capture = piece_t::rook;
					break;
				}

				pieces = pos.get_bitboard<piece_t::queen >(to_move)
							& attackers[to_move];

				if (pieces)
				{
					last_capture = piece_t::queen;
					break;
				}

				pieces = pos.get_bitboard< piece_t::king >(to_move)
							& attackers[to_move];

				if (pieces)
				{
					last_capture = piece_t::king;
					break;
				}

			} while (0);

#ifdef SEE_TEST
			record.moved.push_back( last_capture );
#endif

			/*
			 * Add X-ray attackers
			 */
			if (last_capture == piece_t::knight ||
				last_capture == piece_t::king)
			{
				/*
				 * Clear the least valuable attacker
				 */
				pieces &= -pieces;
				attackers[to_move] &= ~pieces;
			}
			else
			{
				const square_t from =
					static_cast<square_t>(msb64(pieces));
				uint64 new_attackers;

				const bool queen_attacks_diag =
					last_capture == piece_t::queen &&
						(tables.directions[from][square] ==
							direction_t::along_a1h8 ||
						 tables.directions[from][square] ==
						 	direction_t::along_h1a8);

				if (last_capture == piece_t::pawn ||
					last_capture == piece_t::bishop ||
					queen_attacks_diag)
				{
					new_attackers =
						pos.attacks_from<piece_t::bishop>(
							from, occupied)
								& tables.ray_extend[from][square]
									& bishops_queens;

					/*
					 * Prevent reusing the same attacker
					 */
					clear_bit64(from, bishops_queens);
				}
				else
				{
					new_attackers =
						pos.attacks_from< piece_t::rook >(
							from, occupied)
								& tables.ray_extend[from][square]
									& rooks_queens;

					/*
					 * Prevent reusing the same attacker
					 */
					clear_bit64(from, rooks_queens);
				}

				clear_bit64(from, occupied);

				/*
				 * Avoid tagging the piece sitting on
				 * the capture square:
				 */
				clear_bit64(square, new_attackers);

				/*
				 * New attackers have been uncovered. There can be
				 * no more than two, and they may belong to us or the
				 * opponent (or both). Sort out who they belong to
				 * and update each side's attackers
				 */

				const player_t opponent = flip(to_move);

				const uint64 new_attacker_0 =
					new_attackers & pos.get_occupied(to_move );
				const uint64 new_attacker_1 =
					new_attackers & pos.get_occupied(opponent);

				attackers[to_move ] |= new_attacker_0;
				attackers[opponent] |= new_attacker_1;

				/*
				 * Clear the least valuable attacker
				 */
				attackers[to_move] &= occupied;
			}

			to_move = flip(to_move);
		}

		/*
		 * Compute the optimal score via negamax propagation of
		 * the best score up to the root, i.e. scores[1]. This tree
		 * looks like a binary tree where at every node we either
		 * capture or choose not to
		 */
		for (int i = n_moves; i > 1; --i)
		{
			scores[i-1] = -std::max(-scores[i-1],
				scores[i]);
		}

		return scores[1];
	}
}

#endif
