#ifndef __EVAL_H__
#define __EVAL_H__

#include "Position4.h"

namespace Chess
{
	template <player_t P>
	uint16 evaluate_pawns(const Position& pos)
	{
		return 0;
	}

	inline uint16 evaluate_material(const Position& pos)
	{
		return pos.get_material(player_t::white) -
			   pos.get_material(player_t::black);
	}

	/**
	 * @todo Mobility of sliding pieces is somewhat inaccurate:
	 *
	 * 1. If surrounded by all slides, it is immobile, but has a non-
	 *    zero mobility due to popCnt(attacks_from)
	 * 2. A bishop's mobility may be hampered by an enemy pawn,
	 *    but that isn't accounted for here
	 */
	inline uint16 evaluate_mobility(const Position& pos)
	{
		static_assert(player_t::white + player_t::black == 1, "");

		uint16 score[2] = {};

		for (auto side : {player_t::white, player_t::black})
		{
			const uint64 target = ~pos.get_occupied(side);

			// 1. Compute knight mobility

			uint64 pieces = pos.get_bitboard <piece_t::knight>(side);
			while (pieces)
			{
				const square_t from =
					static_cast<square_t>(msb64(pieces));

				const uint64 mobility =
					pos.attacks_from<piece_t::knight>(from) & target;

				score[side] += pop_cnt64(mobility);

				clear_bit64(from, pieces);
			}

			// 2. Compute bishop mobility

			pieces = pos.get_bitboard <piece_t::bishop>(side);
			while (pieces)
			{
				const square_t from =
					static_cast<square_t>(msb64(pieces));

				score[side] +=
					pos.get_mobility<piece_t::bishop>( from );

				clear_bit64(from, pieces);
			}

			// 3. Compute rook mobility

			pieces = pos.get_bitboard <piece_t::rook>(side);
			while (pieces)
			{
				const square_t from =
					static_cast<square_t>(msb64(pieces));

				score[side] +=
					pos.get_mobility< piece_t::rook >( from );

				clear_bit64(from, pieces);
			}

			// 4. Compute queen mobility

			pieces = pos.get_bitboard <piece_t::queen>(side);
			while (pieces)
			{
				const square_t from =
					static_cast<square_t>(msb64(pieces));

				score[side] +=
					pos.get_mobility<piece_t::queen >( from );

				clear_bit64(from, pieces);
			}
		}

		return score[player_t::white] -
			   score[player_t::black];
	}

	inline uint16 evaluate(const Position& pos)
	{
		return evaluate_material(pos)
				+ evaluate_mobility(pos);
	}
}

#endif
