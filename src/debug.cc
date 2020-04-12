/**
 *  \file   debug.cc
 *  \author Jason Fernandez
 *  \date   04/04/2020
 */

#include "chess/debug.h"
#include "chess/util.h"

namespace chess {
namespace debug {

/**
 * Return the given 64-bit integer as an 8x8 bit array
 *
 * @param[in] board The bitboard
 */
std::string PrintBitBoard(std::uint64_t board)
{
    std::string out;

    int prev_rank = 8;
    for (int square = 63; square >= -1; square--) {
        if (util::GetRank(square) != prev_rank) {
            out += "\n ---+---+---+---+---+---+---+--- \n";
            if (square == -1) break;

            prev_rank = util::GetRank(square);
        }

        if (board & (std::uint64_t(1) << square)) {
            out += "| * ";
        } else {
            out += "|   ";
        }

        if (square % 8 == 0) out += "|";
    }

    return (out + "\n");
}

}  // namespace debug
}  // namespace chess
