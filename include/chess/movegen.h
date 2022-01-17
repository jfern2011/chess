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
template <Player player>
void GenerateCaptures(const Position& pos, MoveList* moves);

template <Player player>
void GenerateCheckEvasions(const Position& pos, MoveList* moves);
	
template <Player player>
void GenerateChecks(const Position& pos, MoveList* moves);

template <Player player>
void GenerateLegalMoves(const Position& pos, MoveList* moves);

template <Player player>
void GenerateNonCaptures(const Position& pos, MoveList* moves);

template <Player player>
bool ValidateMove(const Position& pos, std::int32_t move);

}  // namespace chess

#endif  // CHESS_MOVEGEN_H_
