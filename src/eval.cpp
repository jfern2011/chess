#include "eval.h"

namespace Chess
{
    static_assert(player_t::black == 0, "");
    static_assert(player_t::white == 1, "");

    const int16 knight_square_bonus[2][65] =
    {
        { 0,  5,  5, 10, 10,  5,  5, 0,
          0, 10, 15, 20, 20, 15, 10, 0,
          0, 10, 20, 40, 40, 20, 10, 0,
          0, 10, 20, 40, 40, 20, 10, 0,
          0,  5, 20, 30, 30, 20,  5, 0,
          0,  0,  5, 10, 10,  5,  0, 0,
          0,  0,  0,  0,  0,  0,  0, 0,
          0,  0,  0,  0,  0,  0,  0, 0 },

        { 0,  0,  0,  0,  0,  0,  0, 0,
          0,  0,  0,  0,  0,  0,  0, 0,
          0,  0,  5, 10, 10,  5,  0, 0,
          0,  5, 20, 30, 30, 20,  5, 0,
          0, 10, 20, 40, 40, 20, 10, 0,
          0, 10, 20, 40, 40, 20, 10, 0,
          0, 10, 15, 20, 20, 15, 10, 0,
          0,  5,  5, 10, 10,  5,  5, 0 }
    };

    const int16 pawn_square_bonus[2][65] = 
    {
        {  0,  0,  0,  0,  0,  0,  0,  0,
          35, 70, 70, 70, 70, 70, 70, 35,
          25, 50, 50, 50, 50, 50, 50, 25,
          10, 20, 20, 20, 20, 20, 20, 10,
           5, 10, 10, 10, 10, 10, 10,  5,
           1,  2,  2,  2,  2,  2,  2,  1,
           0,  0,  0,  0,  0,  0,  0,  0,
           0,  0,  0,  0,  0,  0,  0,  0 },

        {  0,  0,  0,  0,  0,  0,  0,  0,
           0,  0,  0,  0,  0,  0,  0,  0,
           1,  2,  2,  2,  2,  2,  2,  1,
           5, 10, 10, 10, 10, 10, 10,  5,
          10, 20, 20, 20, 20, 20, 20, 10,
          25, 50, 50, 50, 50, 50, 50, 25,
          35, 70, 70, 70, 70, 70, 70, 35,
           0,  0,  0,  0,  0,  0,  0,  0 }
    };

    int16 evaluate(const Position& pos)
    {
        return evaluate_knights (pos) +
               evaluate_material(pos) +
               evaluate_mobility(pos) +
               evaluate_pawns(pos);
    }

    int16 evaluate_knights (const Position& pos)
    {
        int16 score[2] = {};

        for (auto side : {player_t::white, player_t::black})
        {
            uint64 pieces =
                pos.get_bitboard <piece_t::knight>(side);
            while (pieces)
            {
                const square_t from =
                    static_cast<square_t>(msb64(pieces));

                score[side] += knight_square_bonus[side][from];

                clear_bit64(from, pieces);
            }
        }

        return score[player_t::white] -
               score[player_t::black];
    }

    int16 evaluate_material(const Position& pos)
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
    int16 evaluate_mobility(const Position& pos)
    {
        static_assert(player_t::white + player_t::black == 1, "");

        int16 score[2] = {};

        for (auto side : {player_t::white, player_t::black})
        {
            // 1. Compute bishop mobility

            uint64 pieces =
                pos.get_bitboard <piece_t::bishop>(side);
            while (pieces)
            {
                const square_t from =
                    static_cast<square_t>(msb64(pieces));

                score[side] +=
                    pos.get_mobility<piece_t::bishop>( from );

                clear_bit64(from, pieces);
            }

            // 2. Compute rook mobility

            pieces = pos.get_bitboard <piece_t::rook>(side);
            while (pieces)
            {
                const square_t from =
                    static_cast<square_t>(msb64(pieces));

                score[side] +=
                    pos.get_mobility< piece_t::rook >( from );

                clear_bit64(from, pieces);
            }

            // 3. Compute queen mobility

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

    int16 evaluate_pawns(const Position& pos)
    {
        int16 score[2] = {};

        for (auto side : {player_t::white, player_t::black})
        {
            uint64 pieces =
                pos.get_bitboard < piece_t::pawn >(side);
            while (pieces)
            {
                const square_t from =
                    static_cast<square_t>(msb64(pieces));

                score[side] += pawn_square_bonus[side][from];

                clear_bit64(from, pieces);
            }
        }

        return score[player_t::white] -
               score[player_t::black];
    }
}
