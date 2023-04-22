/**
 *  \file   evaluate.h
 *  \author Jason Fernandez
 *  \date   04/15/2023
 */

#ifndef CHESS_EVALUATE_H_
#define CHESS_EVALUATE_H_

#include "chess/chess.h"
#include "chess/position.h"

namespace chess {
Result GameResult(const Position& pos);

}  // namespace chess

#endif  // CHESS_EVALUATE_H_
