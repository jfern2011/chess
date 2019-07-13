#ifndef __EVAL_H__
#define __EVAL_H__

#include "Position4.h"

namespace Chess
{
    int16 evaluate         (const Position& pos);

    int16 evaluate_material(const Position& pos);

    int16 evaluate_mobility(const Position& pos);

    int16 evaluate_pawns   (const Position& pos);
}

#endif
