/**
 *  \file   interactive.cc
 *  \author Jason Fernandez
 *  \date   11/25/2022
 */

#include "chess/interactive.h"

#include <string>
#include <vector>

#include "superstring/superstring.h"

#include "chess/movegen.h"
#include "chess/util.h"

namespace chess {
/**
 * @brief Resolve a user move. Attempts to parse both coordinate and
 * standard algebraic notation
 *
 * @param pos  The position from which to make the move
 * @param move The desired move
 *
 * @return The bit-packed move, or 0x0 if the move is invalid
 */
std::uint32_t ResolveMove(const Position& pos, const std::string& move) {
    std::array<std::uint32_t, 256> moves;
    const std::size_t n_moves =
        pos.ToMove() == Player::kWhite ?
            GenerateLegalMoves<Player::kWhite>(pos, moves.data()) :
            GenerateLegalMoves<Player::kBlack>(pos, moves.data());

    // First, try coordinate notation

    const jfern::superstring move_ss(move);

    if (move.size() >= 4u) {
        const std::vector<std::string> tokens = move_ss.split(2);

        const Square parsed_from = util::StrToSquare(tokens[0]);
        const Square parsed_to   = util::StrToSquare(tokens[1]);

        for (std::size_t i = 0; i < n_moves; i++) {
            const std::uint32_t mv = moves[i];
            if (util::ExtractFrom(mv) == parsed_from &&
                util::ExtractTo  (mv) == parsed_to) {
                const Piece promoted = util::CharToPiece(move.back());
                if (util::ExtractPromoted(mv) == promoted) {
                    return mv;
                }
            }
        }
    }

    // If we got here, coordinate notation was not used
    // todo...

    // Default return value
    return kBadMove;
}

}  // namespace chess
