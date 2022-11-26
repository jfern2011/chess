/**
 *  \file   interactive.h
 *  \author Jason Fernandez
 *  \date   11/25/2022
 */

#ifndef CHESS_INTERACTIVE_H_
#define CHESS_INTERACTIVE_H_

#include <cstdint>
#include <string>

#include "chess/position.h"

namespace chess {
std::uint32_t ResolveMove(const Position& pos, const std::string& move);

}  // namespace chess

#endif  // CHESS_INTERACTIVE_H_
