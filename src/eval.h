#ifndef __EVAL_H__
#define __EVAL_H__

#include "chess4.h"
#include "chess_util4.h"
#include "Position4.h"

namespace Chess
{
    extern const int16 knight_square_bonus[2][65];
    extern const int16 pawn_square_bonus[2][65];

    int16 evaluate         (const Position& pos);

    int16 evaluate_knights (const Position& pos);

    int16 evaluate_material(const Position& pos);

    int16 evaluate_mobility(const Position& pos);

    int16 evaluate_pawns   (const Position& pos);
}

#endif
