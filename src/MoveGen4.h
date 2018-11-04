#ifndef __MOVEGEN__
#define __MOVEGEN__

#include "Position4.h"

namespace Chess
{
	/**
	 * @namespace MoveGen
	 *
	 * Utility functions for generating captures, non-captures, checks,
	 * and check evasions
	 */
	namespace MoveGen
	{
		/**
		 * Generate legal moves from the position. This is common code
		 * used by the other move generators
		 *
		 * @note Generally, this function does not get called directly
		 *       but rather indirectly by the other move generators.
		 *       This does not handle castling or pawn moves since those
		 *       are specific to the king and pawn, respectively
		 *
		 * @note This function does not correctly handle cases where the
		 *       king is in check; see generate_check_evasions()
		 *
		 * @param[in]  pos      The current position
		 * @param[in]  target   The target squares to move to
		 * @param[in]  pinned   Pinned (potentially unmovable) pieces
		 * @param[out] moves    The list of legal moves
		 * @param[in]  gen_king If true, generate king moves
		 *
		 * @return The number of moves found
		 */
		inline size_t generate(const Position& pos, const uint64 target,
			const uint64 pinned, int32* moves, bool gen_king=true)
		{
			const player_t to_move = pos.get_turn();

			const square_t king_square = pos.get_king_square(to_move);

			const auto& tables =  DataTables::get();

			size_t count = 0;

			/*
			 * Generate knight moves
			 */
			uint64 pieces = pos.get_bitboard<piece_t::knight>(to_move)
								& (~pinned);

			while (pieces)
			{
				const square_t from =
					static_cast<square_t>(msb64(pieces));

				uint64 _moves       =
					tables.knight_attacks[from] & target;

				while (_moves)
				{
					const square_t to =
						static_cast<square_t>(msb64(_moves));

					moves[count++] = pack_move(pos.piece_on(to),
											   from, piece_t::knight,
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
					switch (tables.directions[from][king_square])
					{
					case direction_t::along_a1h8:
					case direction_t::along_h1a8:
						clear_bit64(from, pieces);
						continue;
					case direction_t::along_rank:
						restrict_attacks =
							tables.ranks64[from];
						break;
					default:
						restrict_attacks =
							tables.files64[from];
					}
				}

				uint64 _moves = pos.attacks_from< piece_t::rook >(from)
								 & target & restrict_attacks;

				while (_moves)
				{
					const square_t to =
						static_cast<square_t>(msb64(_moves));

					moves[count++] = pack_move(pos.piece_on(to),
											   from, piece_t::rook,
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
					switch (tables.directions[from][king_square])
					{
					case direction_t::along_file:
					case direction_t::along_rank:
						clear_bit64(from, pieces);
						continue;
					case direction_t::along_a1h8:
						restrict_attacks =
							tables.a1h8_64[from];
						break;
					default:
						restrict_attacks =
							tables.h1a8_64[from];
					}
				}

				uint64 _moves = pos.attacks_from<piece_t::bishop>(from)
								 & target & restrict_attacks;

				while (_moves)
				{
					const square_t to =
						static_cast<square_t>(msb64(_moves));

					moves[count++] = pack_move(pos.piece_on(to),
											   from, piece_t::bishop,
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
				 * If the queen is pinned, then restrict its motion
				 * to along the direction of the pin:
				 */
				uint64 restrict_attacks = ~0;

				if (tables.set_mask[from] & pinned)
				{
					switch (tables.directions[from][king_square])
					{
					case direction_t::along_a1h8:
						restrict_attacks =
							tables.a1h8_64[from];
						break;
					case direction_t::along_h1a8:
						restrict_attacks =
							tables.h1a8_64[from];
						break;
					case direction_t::along_rank:
						restrict_attacks =
							tables.ranks64[from];
						break;
					default:
						restrict_attacks =
							tables.files64[from];
					}
				}

				uint64 _moves = pos.attacks_from< piece_t::queen >(from)
								 & target & restrict_attacks;

				while (_moves)
				{
					const square_t to =
						static_cast<square_t>(msb64(_moves));

					moves[count++] = pack_move(pos.piece_on(to),
											   from, piece_t::queen,
											   piece_t::empty,
											   to);
						
					clear_bit64(to, _moves);
				}

				clear_bit64(from, pieces);
			}

			/*
			 * Generate king non-castle moves
			 */
			if (gen_king)
			{
				uint64 _moves =
					tables.king_attacks[king_square] & target;

				const player_t opponent = flip(to_move);
				while (_moves)
				{
					const square_t to =
						static_cast<square_t>(msb64(_moves));

					if (!pos.under_attack(to, opponent))
					{
						moves[count++] = pack_move(pos.piece_on(to),
												   king_square,
												   piece_t::king,
												   piece_t::empty,
												   to);
					}

					clear_bit64(to, _moves);
				}
			}

			return count;
		}

		/**
		 * Generates captures from a position, all of which are
		 * strictly legal. Note this also includes pawn promotions
		 *
		 * @note This function does not correctly handle cases where the
		 *       king is in check; see generate_check_evasions()
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

			const player_t opponent = flip(to_move);

			const uint64 pinned = pos.get_pinned_pieces(to_move);

			const uint64 xoccupied  = pos.get_occupied(opponent);

			const square_t king_square =
						pos.get_king_square (to_move);

			size_t count = generate(pos, xoccupied,
									pinned, captures);

			const auto& tables = DataTables::get();

			/*
			 * Generate pawn captures (1)
			 */
			const uint64 pawns =
				pos.get_bitboard<piece_t::pawn>(to_move) ;

			uint64 caps = shift_pawns<7>(pawns, to_move) &
				xoccupied;

			while (caps)
			{
				const square_t to   =
					static_cast <square_t>(msb64(caps));
				const square_t from = tables.minus_7[to_move][to];

				if ((tables.set_mask[from] & pinned) &&
					(tables.directions[from][king_square]
						!= direction_t::along_a1h8))
				{
					clear_bit64(to, caps);
					continue;
				}

				if (tables.set_mask[to] & tables.back_rank[opponent])
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
			caps = shift_pawns<9>(pawns, to_move) &
				xoccupied;

			while (caps)
			{
				const square_t to   =
					static_cast <square_t>(msb64(caps));
				const square_t from = tables.minus_9[to_move][to];

				if ((tables.set_mask[from] & pinned) &&
					(tables.directions[from][king_square]
						!= direction_t::along_h1a8))
				{
					clear_bit64(to, caps);
					continue;
				}

				if (tables.set_mask[to] & tables.back_rank[opponent])
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
							tables.directions[king_square][to] !=
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
						const uint64 temp =
							occupied ^ tables.set_mask[from];

						const square_t vic= tables.minus_8[to_move][to];

						const uint64 rank_attacks =
							pos.attacks_from<piece_t::rook>(vic, temp)
								& tables.ranks64[from];

						const uint64 rooksQueens =
							pos.get_bitboard<piece_t::rook >(opponent) |
							pos.get_bitboard<piece_t::queen>(opponent) ;

						if ((pos.get_bitboard<
								piece_t::king>( to_move ) & rank_attacks)
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
			uint64 promotions = shift_pawns<8>(
					pos.get_bitboard<piece_t::pawn>(to_move), to_move)
						& (~occupied) & tables.back_rank[opponent];

			while (promotions)
			{
				const square_t to   = 
					static_cast<square_t>(msb64(promotions));
				const square_t from =  tables.minus_8[to_move][to];

				if (tables.set_mask[from] & pinned)
				{
					clear_bit64 ( to, promotions );
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
		 * @note This function does not correctly handle cases where the
		 *       king is in check; see generate_check_evasions()
		 *
		 * @note This function does not include promotions, as those are
		 *       handled by \ref generate_captures()
		 *
		 * @param[in]  pos   The input position
		 * @param[out] moves The list of non-captures
		 *
		 * @return The number of moves found
		 */
		inline size_t generate_noncaptures(const Position& pos,
			int32* moves)
		{
			const player_t to_move = pos.get_turn();

			const player_t opponent = flip(to_move);

			const uint64 pinned  =  pos.get_pinned_pieces(to_move);

			const uint64 occupied =
				pos.get_occupied(player_t::white) |
				pos.get_occupied(player_t::black);

			const square_t king_square =
						pos.get_king_square(to_move);

			size_t count = generate(pos, ~occupied, pinned, moves);

			const auto& tables = DataTables::get();

			/*
			 * Generate pawn advances, not including promotions
			 */
			uint64 advances1 = 
				shift_pawns<8>(pos.get_bitboard<piece_t::pawn>(to_move),
					to_move) & (~occupied)
						& (~tables.back_rank[opponent]);

			uint64 advances2
				= shift_pawns<8>(advances1 &  tables._3rd_rank[to_move],
					to_move) & (~occupied);

			while (advances1)
			{
				const square_t to   =
					static_cast<square_t>(msb64(advances1));
				const square_t from = tables.minus_8[to_move][to];

				if ((tables.set_mask[from] & pinned) &&
					(tables.directions[from][king_square]
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
				const square_t to   =
					static_cast<square_t>(msb64(advances2));
				const square_t from = tables.minus_16[to_move][to];

				if ((tables.set_mask[from] & pinned) &&
					(tables.directions[from][king_square]
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
										 opponent)
					&& !pos.under_attack(tables.castle_OO_path[to_move][1],
										 opponent))
				{
						moves[count++] =
							pack_move(piece_t::empty,
									  tables.king_home[to_move], piece_t::king,
									  piece_t::empty,
									  tables.castle_OO_dest[to_move]);
				}
			}

			if (pos.can_castle_long(to_move))
			{
				if (!(occupied & tables.queenside[to_move])
					&& !pos.under_attack(tables.castle_OOO_path[to_move][0],
										 opponent)
					&& !pos.under_attack(tables.castle_OOO_path[to_move][1],
										 opponent))
				{
						moves[count++] =
							pack_move(piece_t::empty,
									  tables.king_home[to_move], piece_t::king,
									  piece_t::empty,
									  tables.castle_OOO_dest[to_move]);
				}
			}

			return count;
		}

		/**
		 * Generate moves that get a king out of check. It's assumed that if
		 * this method is called, the player on move is in check. Note all
		 * generated moves are strictly legal
		 *
		 * @param[in] pos     The current position
		 * @param[out] moves  The set of moves
		 *
		 * @return  The total number of moves that were generated that evade
		 *          check
		 */
		inline size_t generate_check_evasions(const Position& pos,
			int32* moves)
		{
			const uint64 occupied = pos.get_occupied(player_t::white) |
									pos.get_occupied(player_t::black);

			const player_t to_move = pos.get_turn();

			const player_t opponent = flip(to_move);

			const square_t king_square = pos.get_king_square(to_move);

			const auto & tables = DataTables::get();

			size_t count = 0;

			/*
			 * Step 1: Gather all enemy squares attacking our king
			 */
			const uint64 attacks_king =
				pos.attacks_to(king_square, opponent);

			/*
			 * Step 2: Generate king moves that get out of check
			 */
			uint64 _moves =
				pos.attacks_from<piece_t::king>(king_square)
					& (~pos.get_occupied(to_move));

			while (_moves)
			{
				const square_t to =
					static_cast<square_t>(msb64(_moves));
				clear_bit64(to, _moves);

				const uint64 attack_dir =
					tables.ray_extend[king_square][to] & attacks_king;

				const uint64 rooks_queens =
					pos.get_bitboard<piece_t::queen >(opponent) |
					pos.get_bitboard<piece_t::rook  >(opponent);

				const uint64 bishops_queens =
					pos.get_bitboard<piece_t::queen >(opponent) |
					pos.get_bitboard<piece_t::bishop>(opponent);

				/*
				 * This says if we're in check by a sliding piece, then
				 * do not move along the line of attack unless it is to
				 * capture the checking piece
				 */
				if (((attack_dir & rooks_queens) ||
						(attack_dir & bishops_queens))
							&& !(tables.set_mask[to] & attacks_king))
					continue;

				if (!pos.under_attack(to, opponent))
				{
					moves[count++] = pack_move(pos.piece_on(to),
											   king_square, piece_t::king,
											   piece_t::empty,
											   to);
				}
			}

			/*
			 * Step 3a: If the king is attacked twice, we are done
			 */
			if (attacks_king & (attacks_king-1))
		 	   return count;

		 	/*
			 * Step 3b: Otherwise, (1) get the square the attacking piece
			 *          is on (the "to" square for capture moves), and
			 *          (2) a bitboard connecting the king square and the
			 *          attacking piece for interposing moves
			 */
		 	const square_t attacker =
		 			static_cast<square_t>(msb64(attacks_king));
		 	const uint64 target =
		 		  	tables.ray_segment [king_square][attacker];

		 	const uint64 pinned =
		 		pos.get_pinned_pieces(to_move);

		 	const piece_t attacking_piece = pos.piece_on(attacker);

		 	/*
		 	 * Step 4: Generate knight, rook, bishop, and queen moves:
		 	 */
		 	count += generate(pos,
		 					  target | tables.set_mask[attacker],
							  pinned,
							  &moves[count],
							  false);

			/*
			 * Step 5: Generate pawn moves
			 */
			const uint64 pieces =
				pos.get_bitboard<piece_t::pawn>(to_move)
					& (~pinned);

			/*
			 * Step 5a: Generate pawn moves that capture the checking
			 *          piece
			 */
			const uint64 r_caps =
				shift_pawns<7>(pieces, to_move) & attacks_king;
			const uint64 l_caps =
				shift_pawns<9>(pieces, to_move) & attacks_king;

			if (r_caps)
			{
				const square_t from = tables.minus_7[to_move][attacker];

				if (tables.set_mask[attacker] &
						tables.back_rank[opponent])
				{
					moves[count++] = pack_move(attacking_piece,
											   from,
											   piece_t::pawn,
											   piece_t::knight,
											   attacker);

					moves[count++] = pack_move(attacking_piece,
											   from,
											   piece_t::pawn,
											   piece_t::bishop,
											   attacker);

					moves[count++] = pack_move(attacking_piece,
											   from,
											   piece_t::pawn,
											   piece_t::rook,
											   attacker);

					moves[count++] = pack_move(attacking_piece,
											   from,
											   piece_t::pawn,
											   piece_t::queen,
											   attacker);
				}
				else
				{
					moves[count++] = pack_move(attacking_piece,
											   from,
											   piece_t::pawn,
											   piece_t::empty,
											   attacker);
				}
			}

			if (l_caps)
			{
				const square_t from = tables.minus_9[to_move][attacker];

				if (tables.set_mask[attacker] &
						tables.back_rank[opponent])
				{
					moves[count++] = pack_move(attacking_piece,
											   from,
											   piece_t::pawn,
											   piece_t::knight,
											   attacker);

					moves[count++] = pack_move(attacking_piece,
											   from,
											   piece_t::pawn,
											   piece_t::bishop,
											   attacker);

					moves[count++] = pack_move(attacking_piece,
											   from,
											   piece_t::pawn,
											   piece_t::rook,
											   attacker);

					moves[count++] = pack_move(attacking_piece,
											   from,
											   piece_t::pawn,
											   piece_t::queen,
											   attacker);
				}
				else
				{
					moves[count++] = pack_move(attacking_piece,
											   from,
											   piece_t::pawn,
											   piece_t::empty,
											   attacker);
				}
			}

			/*
			 * Step 5b: Generate en passant captures
			 */
			if (pos.ep_data().target != square_t::BAD_SQUARE
					&& attacking_piece == piece_t::pawn)
			{
				const square_t from1 = pos.ep_data().src[0];
				const square_t from2 = pos.ep_data().src[1];

				const square_t to = pos.ep_data().target;

				if ((from1 != square_t::BAD_SQUARE)
						&& ! (tables.set_mask[from1] & pinned))
				{
					moves[count++] = pack_move(piece_t::pawn,
											   from1,
											   piece_t::pawn,
											   piece_t::empty,
											   to);
				}

				if ((from2 != square_t::BAD_SQUARE)
						&& ! (tables.set_mask[from2] & pinned))
				{
					moves[count++] = pack_move(piece_t::pawn,
											   from2,
											   piece_t::pawn,
											   piece_t::empty,
											   to);
				}
			}

			/*
			 * If we're in check by a knight or pawn then we're
			 * done (it makes no sense to check for interposing
			 * moves here)
			 */
			if (attacking_piece == piece_t::knight ||
				attacking_piece == piece_t::pawn)
				return count;

			/*
			 * Step 5c: Generate interposing pawn moves
			 */
			uint64 advances1 = 
				shift_pawns<8>(pos.get_bitboard<piece_t::pawn>(to_move),
					to_move) & (~occupied);

			uint64 advances2
				= shift_pawns<8>(advances1 & tables._3rd_rank[ to_move],
					to_move) & (~occupied) & target;

			advances1 &= target; // finish computing advances1

		    while (advances1)
		    {
		        const square_t to   =
		        	static_cast<square_t>(msb64(advances1));
		        const square_t from = tables.minus_8[to_move][to];

		        clear_bit64(to, advances1);

		        if (pinned & tables.set_mask[from])
		        	continue;

		        if (tables.set_mask[to] & (rank_8 | rank_1))
		        {
		        	moves[count++] = pack_move(piece_t::empty,
											   from,
											   piece_t::pawn,
											   piece_t::knight,
											   to);

					moves[count++] = pack_move(piece_t::empty,
											   from,
											   piece_t::pawn,
											   piece_t::bishop,
											   to);

					moves[count++] = pack_move(piece_t::empty,
											   from,
											   piece_t::pawn,
											   piece_t::rook,
											   to);

					moves[count++] = pack_move(piece_t::empty,
											   from,
											   piece_t::pawn,
											   piece_t::queen,
											   to);
		        }
		        else
		        {
		        	moves[count++] = pack_move(piece_t::empty,
											   from,
											   piece_t::pawn,
											   piece_t::empty,
											   to);
		        }
		    }

		    while (advances2)
		    {
		        const square_t to   =
		        	static_cast<square_t>(msb64(advances2));
		        const square_t from = tables.minus_16[to_move][to];

		        clear_bit64(to, advances2);
		        
		        if (pinned & tables.set_mask[from])
		            continue;
		        
		        moves[count++] = pack_move(piece_t::empty,
		        						   from, piece_t::pawn,
		        						   piece_t::empty,
		        						   to);
		    }

		    return count;
		}

		/**
		 * Generate a set of strictly legal moves that deliver check, but are
		 * neither captures nor pawn promotions since those are already
		 * generated in \ref generate_captures()
		 *
		 * @param[in] pos     The current position
		 * @param[out] moves  The set of checks
		 *
		 * @return The total number of moves that were generated that deliver
		 *         check
		 */
		inline size_t generate_checks(const Position& pos,
			int32* moves)
		{
			const uint64 occupied = pos.get_occupied(player_t::white) |
									pos.get_occupied(player_t::black);

			const player_t to_move = pos.get_turn();
			const player_t opponent = flip(to_move);

			const square_t king_square  = pos.get_king_square(to_move);
			const square_t xking_square = pos.get_king_square(opponent);

			const uint64 target = ~occupied;

			const auto& tables = DataTables::get();

			const uint64 pinned  = pos.get_pinned_pieces(to_move);
			const uint64 xpinned = pos.get_discover_ready(opponent);

			size_t count = 0;

			/*
			 * 1. Generate pawn non-captures and non-promotions that uncover
			 *    check:
			 */

			/*
			 * 1.1 Generate discovered checks:
			 */
			const uint64 candidates =
				pos.get_bitboard<piece_t::pawn>(to_move) & xpinned;

			uint64 advances1 = shift_pawns<8>( candidates, to_move)
								& (~tables.back_rank[opponent])
								& (~occupied);

			uint64 advances2 = shift_pawns<8>(
				advances1 & tables._3rd_rank[to_move], to_move)
					& (~occupied);

			while (advances1)
			{
				const square_t to   =
					static_cast<square_t>(msb64(advances1));
				const square_t from = tables.minus_8[to_move][to];

				/*
				 * Don't include this move if (1) our pawn in pinned and
				 * we're not moving in the "pin" direction, or (2) the
				 * opponent's king is on the same file as the moved pawn,
				 * which cannot possibly result in a discovered check:
				 */
				if (((tables.set_mask[from] & pinned) &&
					 tables.directions[from][king_square ] !=
						direction_t::along_file) ||
					(tables.directions[from][xking_square] ==
						direction_t::along_file))
				{
					clear_bit64(to, advances1);
					continue;
				}
				else
				{
					moves[count++] = pack_move(piece_t::empty,
											   from,
											   piece_t::pawn,
											   piece_t::empty,
											   to);
				}

				clear_bit64(to, advances1);
			}

			while (advances2)
			{
				const square_t to   =
					static_cast<square_t>(msb64(advances2));
				const square_t from = tables.minus_16[to_move][to];

				/*
				 * Don't include this move if (1) our pawn in pinned and
				 * we're not moving in the "pin" direction, or (2) the
				 * opponent's king is on the same file as the moved pawn,
				 * which cannot possibly result in a discovered check:
				 */
				if (((tables.set_mask[from] & pinned) &&
					 tables.directions[from][king_square ] !=
						direction_t::along_file) ||
					(tables.directions[from][xking_square] ==
						direction_t::along_file))
				{
					clear_bit64(to, advances2);
					continue;
				}
				else
				{
					moves[count++] = pack_move(piece_t::empty,
											   from,
											   piece_t::pawn,
											   piece_t::empty,
											   to);
				}

				clear_bit64(to, advances2);
			}

			/*
			 * 1.2 Generate direct checks:
			 */
			const uint64 attack_mask =
				tables.pawn_attacks[opponent][xking_square];

			advances1 =
				shift_pawns<8>(
					pos.get_bitboard< piece_t::pawn >(to_move), to_move)
						& (~occupied) & (~tables.back_rank[opponent]);

			advances2 =
				shift_pawns<8>( advances1 & tables._3rd_rank[to_move],
					to_move) & (~occupied);

			advances1 &= attack_mask;
			advances2 &= attack_mask;

			while (advances1)
			{
				const square_t to   =
					static_cast<square_t>(msb64(advances1));
				const square_t from = tables.minus_8[to_move][to];

				if ((tables.set_mask[from] & pinned) &&
					(tables.directions[from][king_square] !=
						direction_t::along_file))
				{
					clear_bit64(to, advances1);
					continue;
				}
				else
				{
					moves[count++] = pack_move(piece_t::empty,
											   from,
											   piece_t::pawn,
											   piece_t::empty,
											   to);
				}

				clear_bit64(to, advances1);
			}

			while (advances2)
			{
				const square_t to   =
					static_cast<square_t>(msb64(advances2));
				const square_t from = tables.minus_16[to_move][to];

				if ((tables.set_mask[from] & pinned) &&
					(tables.directions[from][king_square] !=
						direction_t::along_file))
				{
					clear_bit64(to, advances2);
					continue;
				}
				else
				{
					moves[count++] = pack_move(piece_t::empty,
											   from,
											   piece_t::pawn,
											   piece_t::empty,
											   to);
				}

				clear_bit64(to, advances2);
			}

			/*
			 * 2.1 Generate knight non-captures that deliver discovered
			 *     check
			 */
			uint64 pieces = pos.get_bitboard<piece_t::knight >(to_move)
								& xpinned & (~pinned);
			while (pieces)
			{
				const square_t from =
					static_cast<square_t>(msb64(pieces));

				uint64 attacks  = tables.knight_attacks[from] & target;
				while (attacks)
				{
					const square_t to =
						static_cast<square_t>(msb64(attacks));
					moves[count++] = pack_move(piece_t::empty,
											   from,
											   piece_t::knight,
											   piece_t::empty,
											   to);

					clear_bit64(to, attacks);
				}

				clear_bit64(from, pieces);
			}

			/*
			 * 2.2 Generate knight non-captures that deliver direct
			 *     check
			 */
			pieces = pos.get_bitboard<piece_t::knight>(to_move)
						& ~xpinned & (~pinned);

			const uint64 attacksTo = tables.knight_attacks[xking_square];
			while (pieces)
			{
				const square_t from =
					static_cast<square_t>(msb64(pieces));
				uint64 attacks =
						tables.knight_attacks[from] & target & attacksTo;

				while (attacks)
				{
					const square_t to =
						static_cast<square_t>(msb64(attacks));
					moves[count++] = pack_move(piece_t::empty,
											   from,
											   piece_t::knight,
											   piece_t::empty,
											   to);

					clear_bit64(to, attacks);
				}

				clear_bit64(from, pieces);
			}

			/*
			 * 3.1 Generate king non-captures that deliver discovered
			 *     check
			 */
			pieces = pos.get_bitboard<piece_t::king>(to_move) & xpinned;
			while (pieces)
			{
				const square_t from =
					static_cast<square_t>(msb64(pieces));

				uint64 attacks = tables.king_attacks[from] & target;

				while (attacks)
				{
					const square_t to
							= static_cast<square_t>(msb64(attacks));

					if (pos.under_attack(to, opponent) ||
						tables.directions[to][king_square] ==
						    tables.directions[king_square][xking_square])
					{
						clear_bit64(to, attacks);
						continue;
					}
					moves[count++] = pack_move(piece_t::empty,
											   from,
											   piece_t::king,
											   piece_t::empty,
											   to);

					clear_bit64(to, attacks);
				}

				clear_bit64(from, pieces);
			}

			/*
			 * 3.2 Generate castle moves that deliver direct check
			 */ 
			if (pos.can_castle_short(to_move))
			{
				if (!(occupied & tables.kingside[to_move])
					&& !pos.under_attack(tables.castle_OO_path[to_move][0],
										 opponent)
					&& !pos.under_attack(tables.castle_OO_path[to_move][1],
										 opponent))
				{
					const square_t F_ = to_move == player_t::white ?
											square_t::F1 : square_t::F8;

					if (pos.attacks_from<piece_t::rook>(F_,
						occupied ^ pos.get_bitboard<piece_t::king>(to_move))
							& pos.get_bitboard<piece_t::king>(opponent))
					{
						moves[count++] =
							pack_move(piece_t::empty,
									  tables.king_home[to_move], piece_t::king,
									  piece_t::empty,
									  tables.castle_OO_dest[to_move]);
					}
				}
			}

			if (pos.can_castle_long(to_move))
			{
				if (!(occupied & tables.queenside[to_move])
					&& !pos.under_attack(tables.castle_OOO_path[to_move][0],
										 opponent)
					&& !pos.under_attack(tables.castle_OOO_path[to_move][1],
										 opponent))
				{
					const square_t D_ = to_move == player_t::white ?
											square_t::D1 : square_t::D8;

					if (pos.attacks_from<piece_t::rook>(D_,
						occupied ^ pos.get_bitboard<piece_t::king>(to_move))
							& pos.get_bitboard<piece_t::king>(opponent))
					{
						moves[count++] =
							pack_move(piece_t::empty,
									  tables.king_home[to_move], piece_t::king,
									  piece_t::empty,
									  tables.castle_OOO_dest[to_move]);
					}
				}
			}

			/*
			 * 4.1 Generate bishop non-captures that deliver discovered
			 *     check
			 */
			pieces = pos.get_bitboard<piece_t::bishop>(to_move) & xpinned;
			while (pieces)
			{
				const square_t from =
					static_cast<square_t>(msb64(pieces));

				/*
				 * If the bishop is pinned along a file or rank then we cannot
				 * move it, so don't bother generating an attacks_from
				 * bitboard. If it's pinned along an a1-h8 diagonal, clear the
				 * h1-a8 bits of its attacks_from bitboard to ensure we only
				 * keep moves along the direction of the pin (similar idea for
				 * when pinned along an h1-a8 direction):
				 */
				uint64 restrictAttacks = ~0;

				if (tables.set_mask[from] & pinned)
				{
					switch (tables.directions[from][king_square])
					{
					case direction_t::along_file:
					case direction_t::along_rank:
						clear_bit64(from, pieces);
						continue;
					case direction_t::along_a1h8:
						restrictAttacks = tables.a1h8_64[from];
						break;
					default:
						restrictAttacks = tables.h1a8_64[from];
					}
				}

				uint64 attacks =
					pos.attacks_from<piece_t::bishop>(from) & target
						& restrictAttacks;

				while (attacks)
				{
					const square_t to =
						static_cast<square_t>(msb64(attacks));

					moves[count++] = pack_move(piece_t::empty,
											   from,
											   piece_t::bishop,
											   piece_t::empty,
											   to);

					clear_bit64(to, attacks);
				}

				clear_bit64(from, pieces);
			}

			/*
			 * 4.2 Generate bishop non-captures that deliver direct
			 *     check
			 */
			const uint64 diagTarget = 
					pos.attacks_from<piece_t::bishop>(xking_square);

			pieces = pos.get_bitboard<
				piece_t::bishop>( to_move ) & (~xpinned);

			while (pieces)
			{
				const square_t from
					= static_cast<square_t>(msb64(pieces));

				/*
				 * If the bishop is pinned along a file or rank then we cannot
				 * move it, so don't bother generating an attacks_from
				 * bitboard. If it's pinned along an a1-h8 diagonal, clear the
				 * h1-a8 bits of its attacks_from bitboard to ensure we only
				 * keep moves along the direction of the pin (similar idea for
				 * when pinned along an h1-a8 direction):
				 */
				uint64 restrictAttacks = ~0;

				if (tables.set_mask[from] & pinned)
				{
					switch (tables.directions[from][king_square])
					{
					case direction_t::along_file:
					case direction_t::along_rank:
						clear_bit64(from, pieces);
						continue;
					case direction_t::along_a1h8:
						restrictAttacks = tables.a1h8_64[from];
						break;
					default:
						restrictAttacks = tables.h1a8_64[from];
					}
				}

				uint64 attacks =
					pos.attacks_from<piece_t::bishop>(from) & target
						& diagTarget & restrictAttacks;

				while (attacks)
				{
					const square_t to =
						static_cast<square_t>(msb64(attacks));
					
					moves[count++] = pack_move(piece_t::empty,
											   from,
											   piece_t::bishop,
											   piece_t::empty,
											   to);

					clear_bit64(to, attacks);
				}

				clear_bit64(from, pieces);
			}

			/*
			 * 5.1 Generate rook non-captures that deliver discovered
			 *     check
			 */
			pieces = pos.get_bitboard<piece_t::rook>(to_move) & xpinned;
			while (pieces)
			{
				const square_t from =
					static_cast<square_t>(msb64(pieces));

				/*
				 * If this rook is pinned along a diagonal, we can't move
				 * it, so don't bother generating an attacks_from bitboard.
				 * If pinned along a rank then clear the file bits of its
				 * attacks_from bitboard to ensure we only keep moves along
				 * the direction of the pin (similar reasoning for when
				 * pinned along a file):
				 */
				uint64 restrictAttacks = ~0;

				if (tables.set_mask[from] & pinned)
				{
					switch (tables.directions[from][king_square])
					{
					case direction_t::along_a1h8:
					case direction_t::along_h1a8:
						clear_bit64(from, pieces);
						continue;
					case direction_t::along_rank:
						restrictAttacks = tables.ranks64[from];
						break;
					default:
						restrictAttacks = tables.files64[from];
					}
				}

				uint64 attacks =
					pos.attacks_from<piece_t::rook>(from) & target
						& restrictAttacks;

				while (attacks)
				{
					const square_t to =
						static_cast<square_t>(msb64(attacks));

					moves[count++] = pack_move(piece_t::empty,
											   from,
											   piece_t::rook,
											   piece_t::empty,
											   to);

					clear_bit64(to, attacks);
				}

				clear_bit64(from, pieces);
			}

			/*
			 * 5.2 Generate rook non-captures that deliver direct
			 *     check
			 */
			const uint64 rookTarget = 
				pos.attacks_from<piece_t::rook>(xking_square);

			pieces = pos.get_bitboard<
				piece_t::rook>(to_move) & (~xpinned);

			while (pieces)
			{
				const square_t from =
					static_cast<square_t>(msb64(pieces));

				/*
				 * If this rook is pinned along a diagonal, we can't move
				 * it, so don't bother generating an attacks_from bitboard.
				 * If pinned along a rank then clear the file bits of its
				 * attacks_from bitboard to ensure we only keep moves along
				 * the direction of the pin (similar reasoning for when
				 * pinned along a file):
				 */
				uint64 restrictAttacks = ~0;

				if (tables.set_mask[from] & pinned)
				{
					switch (tables.directions[from][king_square])
					{
					case direction_t::along_a1h8:
					case direction_t::along_h1a8:
						clear_bit64(from, pieces);
						continue;
					case direction_t::along_rank:
						restrictAttacks = tables.ranks64[from];
						break;
					default:
						restrictAttacks = tables.files64[from];
					}
				}

				uint64 attacks =
					pos.attacks_from<piece_t::rook>(from) & target
						& rookTarget & restrictAttacks;

				while (attacks)
				{
					const square_t to =
						static_cast<square_t>(msb64(attacks));

					moves[count++] = pack_move(piece_t::empty,
											   from,
											   piece_t::rook,
											   piece_t::empty,
											   to);

					clear_bit64(to, attacks);
				}

				clear_bit64(from, pieces);
			}

			/*
			 * 6.  Generate queen non-captures that deliver direct check (queens
			 *     cannot uncover check):
			 */
			const uint64 queenTarget = diagTarget | rookTarget;

			pieces = pos.get_bitboard<piece_t::queen>(to_move);

			while (pieces)
			{
				const square_t from =
					static_cast<square_t>(msb64(pieces));

				/*
				 * If the queen is pinned, then restrict its motion to along the
				 * direction of the pin:
				 */
				uint64 restrictAttacks = ~0;

				if (tables.set_mask[from] & pinned)
				{
					switch (tables.directions[from][king_square])
					{
					case direction_t::along_a1h8:
						restrictAttacks = tables.a1h8_64[from];
						break;
					case direction_t::along_h1a8:
						restrictAttacks = tables.h1a8_64[from];
						break;
					case direction_t::along_rank:
						restrictAttacks = tables.ranks64[from];
						break;
					default:
						restrictAttacks = tables.files64[from];
					}
				}

				uint64 attacks =
					pos.attacks_from<piece_t::queen>(from) & target
						& queenTarget & restrictAttacks;

				while (attacks)
				{
					const square_t to =
						static_cast<square_t>(msb64(attacks));

					moves[count++] = pack_move(piece_t::empty,
											   from,
											   piece_t::queen,
											   piece_t::empty,
											   to);

					clear_bit64(to, attacks);
				}

				clear_bit64(from, pieces);
			}

			return count;
		}

		/**
		 * Verify that the specified move can be played legally from this
		 * position
		 *
		 * @param [in] pos   The current position
		 * @param [in] move  The move to play
		 * @param [in] check True if the side on move is in check
		 *
		 * @return True if the move can be played
		 */
		inline bool validate_move(const Position& pos, int32 move,
			bool check)
		{
			const player_t to_move = pos.get_turn();
			const player_t opponent = flip(to_move);

			const piece_t captured = extract_captured(move);
			const square_t from    = extract_from(move);
			const piece_t moved    = extract_moved(move);
			const square_t to      = extract_to(move);

			const square_t king_square = pos.get_king_square(to_move);

			const auto& tables = DataTables::get();

			/*
			 * Verify that (1) the moved piece exists on the origin square,
			 * (2) we occupy the origin square, and (3) we do not
			 * occupy the destination square:
			 */
			if (pos.piece_on(from) != moved ||
				!(pos.get_occupied(to_move) & tables.set_mask[from]) ||
					(pos.get_occupied(to_move) & tables.set_mask[to]) != 0)
				return false;

			if (check)
			{
				/*
				 * Verify we are not trying to castle while in check:
				 */
				if (moved == piece_t::king && abs(from-to) == 2)
					return false;

				const uint64 attacks_king =
					pos.attacks_to(king_square, opponent);

				if (attacks_king & (attacks_king-1))
				{
					/*
				 	 * If we're in a double check, and we didn't move
				 	 * the king, this move is illegal:
				 	 */
					if (moved != piece_t::king) return false;
				}
				else if (moved != piece_t::king)
				{
					/*
					 * If this move neither captures nor blocks the checking
					 * piece, it is illegal:
					 */
					const square_t attacker =
							static_cast<square_t>( msb64(attacks_king) );
					if (to != attacker && !(tables.set_mask[to]
							& tables.ray_segment[attacker][king_square]))
					{
						return false;
					}
				}
			}

			/*
			 * If this piece is pinned, make sure we're only moving it
			 * along the pin direction
			 */
			direction_t pin_dir = direction_t::none;
			if (moved != piece_t::king)
			{
				pin_dir  =  pos.is_pinned( from, to_move );
				if (pin_dir != direction_t::none &&
					pin_dir != tables.directions[from][to])
					return false;
			}

			const uint64 occupied =
				pos.get_occupied(player_t::white) | 
				pos.get_occupied(player_t::black);

			bool en_passant = false;
			switch (moved)
			{
			case piece_t::pawn:
				if (captured && pos.piece_on(to) == piece_t::empty)
				{
					en_passant = true;
					/*
					 * Check if en passant is playable from the position:
					 */
					if (pos.ep_data().target == to &&
						(pos.ep_data().src[ 0 ] != from
							&& pos.ep_data().src[1] != from))
						return false;

					/*
					 * The capturing pawn isn't pinned but we still want
					 * want to prevent against this sort of thing:
					 *
					 * 4k3/8/8/2KPp1r1/8/8/8/8 w - e6 0 2
					 *
					 * In this case white still can't capture en passant
					 * because of the rook!
					 */
					const uint64 temp =
						occupied ^ tables.set_mask[from];

					const square_t vic = tables.minus_8[to_move][to];

					const uint64 rank_attacks =
						pos.attacks_from<piece_t::rook>(vic,temp)
							& tables.ranks64[from];

					const uint64 rooksQueens =
						pos.get_bitboard<piece_t::rook >(opponent) |
						pos.get_bitboard<piece_t::queen>(opponent);

					if ((rank_attacks &
							pos.get_bitboard<piece_t::king>(to_move)) &&
								(rank_attacks & rooksQueens))
						return false;
				}
				else if (abs(from-to) == 8)
				{
					/*
					 * If this is a pawn advance, make sure the "to" square
					 * is vacant:
					 */
					if (pos.piece_on(to) != piece_t::empty)
						return false;
				}
				else if (abs(from-to) == 16)
				{
					/*
					 * If this is a double pawn advance, make sure both
					 * squares are vacant:
					 */
					const square_t step1 = tables.minus_8[to_move][to];
					if (pos.piece_on(to) != piece_t::empty ||
							pos.piece_on(step1) != piece_t::empty)
						return false;
				}
				break;
			case piece_t::bishop:
			case piece_t::rook:
			case piece_t::queen:
				/*
				 * If this is a sliding piece, make sure there are no
				 * occupied squares between "from" and "to":
				 */
				if (tables.ray_segment[from][to] & occupied)
					return false;
				break;
			case piece_t::king:
				/*
				 * Note that if this is a castling move, we do not need
				 * to check for a rook on its home square as that's
				 * already taken care of in the pos._castle_rights data
				 */
				if (abs(from-to) == 2 && !check)
				{
					if (from > to
							 && pos.can_castle_short( to_move ))
					{
						if ((occupied & tables.kingside[to_move])
							|| !pos.under_attack(tables.castle_OO_path [to_move][0],
												 opponent)
							|| !pos.under_attack(tables.castle_OO_path [to_move][1],
												 opponent))
						{
							return false;
						}
					}
					else if (from < to
								&& pos.can_castle_long(to_move))
					{
						if ((occupied & tables.queenside[to_move])
							|| !pos.under_attack(tables.castle_OOO_path[to_move][0],
												 opponent)
							|| !pos.under_attack(tables.castle_OOO_path[to_move][1],
												 opponent))
						{
							return false;
						}
					}
					else
					{
						/* Can't castle short or long */
						return false;
					}
				}

				/*
				 * Make sure we aren't trying to move the king
				 * into check:
				 */
				else if (pos.under_attack(to, opponent) )
					return false;
			default:
				break;
			}

			/*
			 * If we captured a piece, verify it is on "to" (unless
			 * we played en passant). Note that it isn't worth
			 * checking that the captured piece belongs to the
			 * opponent since we already know we don't have a piece
			 * on the "to" square
			 */
			if (!en_passant && pos.piece_on(to) != captured)
				return false;

			return true;
		}
	}
}

#endif
