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

/**
 * @brief Check if the game has been lost by the specified player
 *
 * @tparam P The player to check for loss
 *
 * @param pos The position to evaluate
 *
 * @return True if the specified player has lost
 */
template <Player P>
bool IsLostBy(const Position& pos) {
    return false;
}

template <>
inline bool IsLostBy<Player::kBlack>(const Position& pos) {
    return GameResult(pos) == Result::kWhiteWon;
}
template <>
inline bool IsLostBy<Player::kWhite>(const Position& pos) {
    return GameResult(pos) == Result::kBlackWon;
}

/**
 * @brief Check if the game has been won by the specified player
 *
 * @tparam P The player to check for victory
 *
 * @param pos The position to evaluate
 *
 * @return True if the specified player has won
 */
template <Player P>
bool IsWonBy(const Position& pos) {
    return false;
}

template <>
inline bool IsWonBy<Player::kBlack>(const Position& pos) {
    return GameResult(pos) == Result::kBlackWon;
}
template <>
inline bool IsWonBy<Player::kWhite>(const Position& pos) {
    return GameResult(pos) == Result::kWhiteWon;
}

}  // namespace chess

#endif  // CHESS_EVALUATE_H_
