/**
 *  \file   movegen.h
 *  \author Jason Fernandez
 *  \date   01/09/2022
 */

#ifndef CHESS_MOVEGEN_H_
#define CHESS_MOVEGEN_H_

#include <cstdint>

#include "chess/movelist.h"
#include "chess/position.h"

namespace chess {
void GenerateCaptures(const Position& pos, MoveList* moves);

void GenerateCheckEvasions(const Position& pos, MoveList* moves);
	
void GenerateChecks(const Position& pos, MoveList* moves);

void GenerateLegalMoves(const Position& pos, MoveList* moves);

void GenerateNonCaptures(const Position& pos, MoveList* moves);

bool ValidateMove(const Position& pos, std::int32_t move);

}  // namespace chess

#endif  // CHESS_MOVEGEN_H_
