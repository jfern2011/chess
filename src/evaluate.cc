/**
 *  \file   evaluate.cc
 *  \author Jason Fernandez
 *  \date   04/15/2023
 */

#include <array>
#include <cstddef>
#include <cstdint>

#include "chess/evaluate.h"
#include "chess/movegen.h"

namespace chess {
/**
 * @brief Check if the specified player has moves left
 *
 * @tparam P The player to evaluate
 *
 * @param pos[in] The current position
 *
 * @return True if the player has playable moves, false otherwise
 */
template <Player P>
bool HasMoves(const Position& pos) {
    std::array<std::uint32_t, kMaxMoves> moves;

    const std::size_t n_moves = pos.InCheck<P>() ?
            GenerateCheckEvasions<P>(pos, moves.data()) :
            GenerateLegalMoves<P>(pos, moves.data());

    return n_moves > 0u;
}

/**
 * @brief Get the final result
 *
 * @param pos[in] The position to evaluate
 *
 * @return The game result
 */
Result GameResult(const Position& pos) {
    if (pos.ToMove() == Player::kBlack) {
        if (HasMoves<Player::kBlack>(pos)) {
            return Result::kGameNotOver;
        } else {
            return pos.InCheck<Player::kBlack>() ?
                    Result::kWhiteWon : Result::kDraw;
        }
    } else {
        if (HasMoves<Player::kWhite>(pos)) {
            return Result::kGameNotOver;
        } else {
            return pos.InCheck<Player::kWhite>() ?
                    Result::kBlackWon : Result::kDraw;
        }
    }
}

}  // namespace chess
