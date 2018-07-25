#ifndef __MOVEGEN__
#define __MOVEGEN__

#include "Position4.h"

namespace Chess
{
	/**
	 * @namespace MoveGen
	 *
	 * Utility functions for generateing captures, non-captures, checks,
	 * and check evasions
	 */
	namespace MoveGen
	{
		/**
		 * Generate legal moves from the position. This is general code
		 * used by both \ref generate_captures() and \ref
		 * generate_non_captures()
		 *
		 * @note Generally, this function should not get called directly
		 *       but rather indirectly by the other move generators.
		 *       This does not handle pawn or king moves as those pieces
		 *       move specially
		 *
		 * @param[in]  pos    The current position
		 * @param[in]  target The target squares to move to
		 * @param[in]  pinned Pinned (potentially unmovable) pieces
		 * @param[out] moves  The list of legal moves
		 *
		 * @return The number of moves found
		 */
		inline size_t generate(const Position& pos, const uint64 target,
			const uint64 pinned, int32* moves)
		{
			const player_t to_move = pos.get_turn();

			size_t count = 0;

			const auto& tables =  DataTables::get();

			/*
			 * Generate knight moves
			 */
			uint64 pieces =
				pos.get_bitboard<piece_t::knight>(to_move) & (~pinned);
			while (pieces)
			{
				const square_t from =
					static_cast<square_t>(msb64(pieces));

				uint64 _moves  = tables.knight_attacks[from] & target;

				while (_moves)
				{
					const square_t to =
						static_cast<square_t>(msb64(_moves));

					moves[count++] = pack_move(pos.piece_on(to),
											   from,
											   piece_t::knight,
											   piece_t::empty,
											   to);

					clear_bit64(to, _moves);
				}

				clear_bit64(from, pieces);
			}

			/*
			 * Generate rook moves
			 */
			pieces = pos.get_bitboard<piece_t::rook>(to_move);
			while (pieces)
			{
				const square_t from =
					static_cast<square_t>(msb64(pieces));

				/*
				 * If this rook is pinned along a diagonal then we can't
				 * move it, so don't bother generating an attacks_from
				 * bitboard. If pinned along a rank, then clear the file
				 * bits of its attacks_from bitboard to ensure we only
				 * keep moves along the direction of the pin (same goes
				 * for when pinned along a file):
				 */
				uint64 restrict_attacks = ~0;

				if (tables.set_mask[from] & pinned)
				{
					switch (tables.directions[from][pos.get_king_square(to_move)])
					{
					case direction_t::along_a1h8:
					case direction_t::along_h1a8:
						clear_bit64(from, pieces);
						continue;
					case direction_t::along_rank:
						restrict_attacks = tables.ranks64[from];
						break;
					default:
						restrict_attacks = tables.files64[from];
					}
				}

				uint64 _moves =
					pos.attacks_from<piece_t::rook>(from) & target
						& restrict_attacks;

				while (_moves)
				{
					const square_t to =
						static_cast<square_t>(msb64(_moves));

					moves[count++] = pack_move(pos.piece_on(to),
											   from,
											   piece_t::rook,
											   piece_t::empty,
											   to);

					clear_bit64(to, _moves);
				}

				clear_bit64(from, pieces);
			}

			/*
			 * Generate bishop moves
			 */
			pieces = pos.get_bitboard<piece_t::bishop>(to_move);
			while (pieces)
			{
				const square_t from =
					static_cast<square_t>(msb64(pieces));

				/*
				 * If the bishop is pinned along a file or rank then
				 * we can't move it, so don't bother generating an
				 * attacks_from bitboard. If pinned along an a1-h8
				 * diagonal, clear the h1-a8 bits of its attacks_from
				 * bitboard to ensure we only keep moves along the
				 * direction of the pin (similar idea for when
				 * pinned along an h1-a8 diagonal):
				 */
				uint64 restrict_attacks = ~0;

				if (tables.set_mask[from] & pinned)
				{
					switch (tables.directions[from][pos.get_king_square(to_move)])
					{
					case direction_t::along_file:
					case direction_t::along_rank:
						clear_bit64(from, pieces);
						continue;
					case direction_t::along_a1h8:
						restrict_attacks = tables.a1h8_64[from];
						break;
					default:
						restrict_attacks = tables.h1a8_64[from];
					}
				}

				uint64 _moves =
					pos.attacks_from<piece_t::bishop>(from) & target
						& restrict_attacks;

				while (_moves)
				{
					const square_t to =
						static_cast<square_t>(msb64(_moves));

					moves[count++] = pack_move(pos.piece_on(to),
											   from,
											   piece_t::bishop,
											   piece_t::empty,
											   to);

					clear_bit64(to, _moves);
				}

				clear_bit64(from, pieces);
			}

			/*
			 * Generate queen moves
			 */
			pieces = pos.get_bitboard<piece_t::queen>(to_move);
			while (pieces)
			{
				const square_t from =
					static_cast<square_t>(msb64(pieces));

				/*
				 * If the queen is pinned, then restrict its motion to along
				 * the direction of the pin:
				 */
				uint64 restrict_attacks = ~0;

				if (tables.set_mask[from] & pinned)
				{
					switch (tables.directions[from][pos.get_king_square(to_move)])
					{
					case direction_t::along_a1h8:
						restrict_attacks = tables.a1h8_64[from];
						break;
					case direction_t::along_h1a8:
						restrict_attacks = tables.h1a8_64[from];
						break;
					case direction_t::along_rank:
						restrict_attacks = tables.ranks64[from];
						break;
					default:
						restrict_attacks = tables.files64[from];
					}
				}

				uint64 _moves =
					pos.attacks_from<piece_t::queen>(from) & target
						& restrict_attacks;

				while (_moves)
				{
					const square_t to =
						static_cast<square_t>(msb64(_moves));

					moves[count++] = pack_move(pos.piece_on(to),
											   from,
											   piece_t::queen,
											   piece_t::empty,
											   to);
						
					clear_bit64(to, _moves);
				}

				clear_bit64(from, pieces);
			}

			/*
			 * Generate king non-castle moves
			 */
			pieces = pos.get_bitboard<piece_t::king>(to_move);

			{
				const square_t from =
					static_cast<square_t>(msb64(pieces));

				uint64 _moves = tables.king_attacks[ from ] & target;

				while (_moves)
				{
					const square_t to =
						static_cast<square_t>(msb64(_moves));

					if (pos.under_attack(to, flip(to_move)))
					{
						clear_bit64( to, _moves);
						continue;
					}

					moves[count++] = pack_move(pos.piece_on(to),
											   from,
											   piece_t::king,
											   piece_t::empty,
											   to);

					clear_bit64(to, _moves);
				}
			}

			return count;
		}

		/**
		 * Generate captures from a position, all of which are strictly
		 * legal. Note this also includes pawn promotions
		 *
		 * @param[in]  pos      The input position
		 * @param[out] captures The list of captures
		 *
		 * @return The number of captures found
		 */
		inline size_t generate_captures(const Position& pos,
			int32* captures)
		{
			const player_t to_move = pos.get_turn();

			const uint64 pinned = pos.get_pinned_pieces(to_move);

			size_t count = generate(pos,
									pos.get_occupied(flip(to_move)),
									pinned,
									captures);

			const auto& tables = DataTables::get();

			/*
			 * Generate pawn captures (1)
			 */
			const uint64 pawns =
				pos.get_bitboard<piece_t::pawn>(to_move);

			uint64 caps = shift_pawns<7>(pawns, to_move)
							& pos.get_occupied(flip(to_move));

			while (caps)
			{
				const square_t to   = static_cast<square_t>(msb64(caps));
				const square_t from = tables.minus_7[to_move][to];

				if ((tables.set_mask[from] & pinned) &&
					(tables.directions[from][pos.get_king_square(to_move)]
						!= direction_t::along_a1h8))
				{
					clear_bit64(to, caps);
					continue;
				}

				if (tables.set_mask[to] & tables.back_rank[to_move])
				{
					captures[count++] = pack_move(pos.piece_on(to),
												  from,
												  piece_t::pawn,
												  piece_t::knight,
												  to);

					captures[count++] = pack_move(pos.piece_on(to),
												  from,
												  piece_t::pawn,
												  piece_t::bishop,
												  to);

					captures[count++] = pack_move(pos.piece_on(to),
												  from,
												  piece_t::pawn,
												  piece_t::rook,
												  to);

					captures[count++] = pack_move(pos.piece_on(to),
												  from,
												  piece_t::pawn,
												  piece_t::queen,
												  to);
				}
				else
				{
					captures[count++] = pack_move(pos.piece_on(to),
												  from,
												  piece_t::pawn,
												  piece_t::empty,
												  to);
				}

				clear_bit64(to, caps);
			}

			/*
			 * Generate pawn captures (2)
			 */
			caps = shift_pawns<9>(pawns, to_move)
					& pos.get_occupied(flip(to_move));

			while (caps)
			{
				const square_t to   = static_cast<square_t>(msb64(caps));
				const square_t from = tables.minus_9[to_move][to];

				if ((tables.set_mask[from] & pinned) &&
					(tables.directions[from][pos.get_king_square(to_move)]
						!= direction_t::along_h1a8))
				{
					clear_bit64(to, caps);
					continue;
				}

				if (tables.set_mask[to] & tables.back_rank[to_move])
				{
					captures[count++] = pack_move(pos.piece_on(to),
												  from,
												  piece_t::pawn,
												  piece_t::knight,
												  to);

					captures[count++] = pack_move(pos.piece_on(to),
												  from,
												  piece_t::pawn,
												  piece_t::bishop,
												  to);

					captures[count++] = pack_move(pos.piece_on(to),
												  from,
												  piece_t::pawn,
												  piece_t::rook,
												  to);

					captures[count++] = pack_move(pos.piece_on(to),
												  from,
												  piece_t::pawn,
												  piece_t::queen,
												  to);
				}
				else
				{
					captures[count++] = pack_move(pos.piece_on(to),
												  from,
												  piece_t::pawn,
												  piece_t::empty,
												  to);
				}

				clear_bit64(to, caps);
			}

			const uint64 occupied =
				pos.get_occupied(player_t::white) |
				pos.get_occupied(player_t::black);

			/*
			 * Generate en passant captures
			 */
			const auto& ep_data = pos.ep_data();

			if (ep_data.target != square_t::BAD_SQUARE)
			{
				for (int i = 0; i < 2; i++)
				{
					const square_t from = ep_data.src[i];
					const square_t to   = ep_data.target;

					if (from == square_t::BAD_SQUARE)
						continue;

					bool is_legal = true;

					/*
					 * If the pawn is pinned, make sure the capture is along
					 * the pin direction:
					 */
					if ((tables.set_mask[from] & pinned) &&
							tables.directions[pos.get_king_square(to_move)][to] !=
								tables.directions[from][to])
					{
						is_legal = false;
					}
					else if (!(tables.set_mask[from] & pinned))
					{
						/*
						 * The capturing pawn isn't pinned but we still want
						 * want to prevent against this sort of thing:
						 *
						 * 4k3/8/8/2KPp1r1/8/8/8/8 w - e6 0 2
						 *
						 * In this case white still can't capture en passant
						 * because of the rook!
						 */
						const uint64 temp = occupied ^ tables.set_mask[from];

						const square_t vic = tables.minus_8[to_move][to];

						const uint64 rank_attacks =
							pos.attacks_from<piece_t::rook>(vic, temp)
								& tables.ranks64[from];

						const uint64 rooksQueens =
							pos.get_bitboard<piece_t::rook >(flip(to_move)) |
							pos.get_bitboard<piece_t::queen>(flip(to_move)) ;

						if ((pos.get_bitboard<piece_t::king>(to_move)
								& rank_attacks)
									&& ( rank_attacks & rooksQueens))
						{
							is_legal = false;
						}
					}

					if (is_legal)
						captures[count++] = pack_move(piece_t::pawn,
													  from,
													  piece_t::pawn,
													  piece_t::empty,
													  to);
				}
			}

			/*
			 * Generate pawn promotions
			 */
			uint64 promotions =
				shift_pawns<8>(
					pos.get_bitboard<piece_t::pawn>(to_move), to_move)
						& (~occupied) & tables.back_rank[ flip(to_move) ];

			while (promotions)
			{
				const square_t to   = static_cast<square_t>(msb64(promotions));
				const square_t from = tables.minus_8[to_move][to];

				if ((tables.set_mask[from] & pinned) &&
					(tables.directions[from][pos.get_king_square(to_move)]
						!= direction_t::along_file))
				{
					clear_bit64(to, promotions);
					continue;
				}

				captures[count++] = pack_move(piece_t::empty,
											  from,
											  piece_t::pawn,
											  piece_t::knight,
											  to);

				captures[count++] = pack_move(piece_t::empty,
											  from,
											  piece_t::pawn,
											  piece_t::bishop,
											  to);

				captures[count++] = pack_move(piece_t::empty,
											  from,
											  piece_t::pawn,
											  piece_t::rook,
											  to);

				captures[count++] = pack_move(piece_t::empty,
											  from,
											  piece_t::pawn,
											  piece_t::queen,
											  to);

				clear_bit64(to, promotions);
			}

			return count;
		}

		/**
		 * Generate non-captures from a position, all of which are
		 * strictly legal
		 *
		 * @param[in]  pos   The input position
		 * @param[out] moves The list of non-captures
		 *
		 * @return The number of moves found
		 */
		inline size_t generate_non_captures(const Position& pos,
			int32* moves)
		{
			const player_t to_move = pos.get_turn();

			const uint64 pinned = pos.get_pinned_pieces(to_move);

			const uint64 occupied =
				pos.get_occupied(player_t::white) |
				pos.get_occupied(player_t::black);

			size_t count = generate(pos,
									~occupied,
									pinned,
									moves);

			const auto& tables = DataTables::get();

			/*
			 * Generate pawn advances, not including promotions (which
			 * is done in generate_captures())
			 */
			uint64 advances1 = 
				shift_pawns<8>(pos.get_bitboard<piece_t::pawn>(to_move),
					to_move) & (~occupied);

			uint64 promotions =
				advances1 & tables.back_rank[flip(to_move)];

			advances1 ^= promotions;

			uint64 advances2
				= shift_pawns<8>(advances1 & tables._3rd_rank[to_move], to_move)
					& (~occupied);

			while (advances1)
			{
				const square_t to   = static_cast<square_t>(msb64(advances1));
				const square_t from = tables.minus_8[to_move][to];

				if ((tables.set_mask[from] & pinned) &&
					(tables.directions[from][pos.get_king_square(to_move)]
						!= direction_t::along_file))
				{
					clear_bit64(to, advances1);
					continue;
				}

				moves[count++] = pack_move(piece_t::empty,
										   from,
										   piece_t::pawn,
										   piece_t::empty,
										   to);

				clear_bit64(to, advances1);
			}

			while (advances2)
			{
				const square_t to   = static_cast<square_t>(msb64(advances2));
				const square_t from = tables.minus_16[to_move][to];

				if ((tables.set_mask[from] & pinned) &&
					(tables.directions[from][pos.get_king_square(to_move)]
						!= direction_t::along_file))
				{
					clear_bit64(to, advances2);
					continue;
				}

				moves[count++] = pack_move(piece_t::empty,
										   from,
										   piece_t::pawn,
										   piece_t::empty,
										   to);

				clear_bit64(to, advances2);
			}

			/*
			 * Generate castle moves
			 */ 
			if (pos.can_castle_short(to_move))
			{
				if (!(occupied & tables.kingside[to_move])
					&& !pos.under_attack(tables.castle_OO_path[to_move][0],
										 flip(to_move))
					&& !pos.under_attack(tables.castle_OO_path[to_move][1],
										 flip(to_move)))
				{
						moves[count++] =
							pack_move(piece_t::empty,
									  tables.king_home[to_move],
									  piece_t::king,
									  piece_t::empty,
									  tables.castle_OO_dest[to_move]);
				}
			}

			if (pos.can_castle_long(to_move))
			{
				if (!(occupied & tables.queenside[to_move])
					&& !pos.under_attack(tables.castle_OOO_path[to_move][0],
										 flip(to_move))
					&& !pos.under_attack(tables.castle_OOO_path[to_move][1],
										 flip(to_move)))
				{
						moves[count++] =
							pack_move(piece_t::empty,
									  tables.king_home[to_move],
									  piece_t::king,
									  piece_t::empty,
									  tables.castle_OOO_dest[to_move]);
				}
			}

			return count;
		}
	}
}

#endif
