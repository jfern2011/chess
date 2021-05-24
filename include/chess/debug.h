/**
 *  \file   debug.h
 *  \author Jason Fernandez
 *  \date   04/04/2020
 */

#ifndef CHESS_DEBUG_H_
#define CHESS_DEBUG_H_

#include <cstdint>
#include <string>

namespace chess {
namespace debug {

std::string PrintBitBoard(std::uint64_t board);
std::string PrintMove(std::int32_t move);

}  // namespace debug
}  // namespace chess

#endif  // CHESS_DEBUG_H_
