/**
 *  \file   static_exchange.h
 *  \author Jason Fernandez
 *  \date   01/14/2023
 */

#ifndef CHESS_STATIC_EXCHANGE_H_
#define CHESS_STATIC_EXCHANGE_H_

#include <cstdint>

#include "chess/attacks.h"
#include "chess/chess.h"
#include "chess/data_tables.h"
#include "chess/position.h"
#include "chess/util.h"

namespace chess {
namespace detail {
/**
 * @brief Select the next piece that can perform a capture
 *
 * @tparam P The player whose turn it is to capture
 *
 * @param position[in]   The position from which to compute SEE
 * @param target[in]     The square over which to play out the capture sequence
 * @param attackers[out] The candidate pieces that can perform the next capture
 * @param defenders[out] The set of defenders belonging to the opponent
 *
 * @return The piece that should perform the next capture
 */
template <Player P>
Piece NextPiece(const Position& position,
                Square target,
                std::uint64_t* attackers,
                std::uint64_t* defenders) noexcept {
    constexpr Player O = util::opponent<P>();

    const std::uint64_t occupied = position.Occupied();

    std::uint64_t temp_attackers = *attackers;

    const auto& player = position.GetPlayerInfo<P>();

    std::uint64_t attacking_pieces = player.Pawns() & temp_attackers;

    if (attacking_pieces != 0u) {
        const auto src = static_cast<Square>(util::Msb(attacking_pieces));

        const std::uint64_t ray = data_tables::kRay[target][src];

        const std::uint64_t attacks_from =
            ray & AttacksFrom<Piece::BISHOP>(src, occupied);

        const std::uint64_t next_attacker =
            attacks_from & (player.Bishops() | player.Queens());

        if (next_attacker != 0u) {
            temp_attackers |= next_attacker;
        } else {
            const auto& opponent = position.GetPlayerInfo<O>();
            const std::uint64_t next_defender =
                attacks_from & (opponent.Bishops() | opponent.Queens());

            *defenders |= next_defender;
        }

        temp_attackers &= data_tables::kClearMask[src];
        *attackers = temp_attackers;

        return Piece::PAWN;
    }

    attacking_pieces = player.Knights() & temp_attackers;

    if (attacking_pieces != 0u) {
        const auto src = static_cast<Square>(util::Msb(attacking_pieces));

        *attackers &= data_tables::kClearMask[src];
        return Piece::KNIGHT;
    }

    attacking_pieces = player.Bishops() & temp_attackers;

    if (attacking_pieces != 0u) {
        const auto src = static_cast<Square>(util::Msb(attacking_pieces));

        const std::uint64_t ray = data_tables::kRay[target][src];

        const std::uint64_t attacks_from =
            ray & AttacksFrom<Piece::BISHOP>(src, occupied);

        const std::uint64_t next_attacker =
            attacks_from & (player.Bishops() | player.Queens());

        if (next_attacker != 0u) {
            temp_attackers |= next_attacker;
        } else {
            const auto& opponent = position.GetPlayerInfo<O>();
            const std::uint64_t next_defender =
                attacks_from & (opponent.Bishops() | opponent.Queens());

            *defenders |= next_defender;
        }

        temp_attackers &= data_tables::kClearMask[src];
        *attackers = temp_attackers;

        return Piece::BISHOP;
    }

    attacking_pieces = player.Rooks() & temp_attackers;

    if (attacking_pieces != 0u) {
        const auto src = static_cast<Square>(util::Msb(attacking_pieces));

        const std::uint64_t ray = data_tables::kRay[target][src];

        const std::uint64_t attacks_from =
            ray & AttacksFrom<Piece::ROOK>(src, occupied);

        const std::uint64_t next_attacker =
            attacks_from & (player.Rooks() | player.Queens());

        if (next_attacker != 0u) {
            temp_attackers |= next_attacker;
        } else {
            const auto& opponent = position.GetPlayerInfo<O>();
            const std::uint64_t next_defender =
                attacks_from & (opponent.Rooks() | opponent.Queens());

            *defenders |= next_defender;
        }

        temp_attackers &= data_tables::kClearMask[src];
        *attackers = temp_attackers;

        return Piece::ROOK;
    }

    attacking_pieces = player.Queens() & temp_attackers;

    if (attacking_pieces != 0u) {
        const auto src = static_cast<Square>(util::Msb(attacking_pieces));

        const std::uint64_t ray = data_tables::kRay[target][src];

        const std::uint64_t attacks_from =
            ray & AttacksFrom<Piece::QUEEN>(src, occupied);

        const bool along_diag =
            data_tables::kDirections[target][src] == Direction::kAlongA1H8 ||
            data_tables::kDirections[target][src] == Direction::kAlongH1A8;

        std::uint64_t next_pieces =
            along_diag ? (player.Queens() | player.Bishops()) :
                         (player.Queens() | player.Rooks());

        const std::uint64_t next_attacker = attacks_from & next_pieces;

        if (next_attacker != 0u) {
            temp_attackers |= next_attacker;
        } else {
            const auto& opponent = position.GetPlayerInfo<O>();

            next_pieces =
                along_diag ? (opponent.Queens() | opponent.Bishops()) :
                             (opponent.Queens() | opponent.Rooks());

            const std::uint64_t next_defender = attacks_from & next_pieces;

            *defenders |= next_defender;
        }

        temp_attackers &= data_tables::kClearMask[src];
        *attackers = temp_attackers;

        return Piece::QUEEN;
    }

    attacking_pieces = player.King() & temp_attackers;

    if (attacking_pieces != 0u) {
        const auto src = static_cast<Square>(util::Msb(attacking_pieces));

        *attackers &= data_tables::kClearMask[src];
        return Piece::KING;
    }

    return Piece::EMPTY;
}

}  // namespace detail

#if 0
// Edge case: enemy bishop attacking bishop attacking enemy pawn on square
// Edge case: two pawns attacking the target
template <Player P>
std::int16_t ComputeSee(const Position& position, Square square) noexcept {
    constexpr Player O = util::opponent<P>();

    const auto& player = position.GetPlayerInfo<P>();

    std::uint64_t occupied = position.Occupied();

    std::uint64_t attackers = player.AttacksTo(square, occupied);

    if (attackers == 0u) {
        // The player on move has no attackers, so we are done
        return 0;
    }

    const auto& opponent = position.GetPlayerInfo<O>();
    std::uint64_t defenders = opponent.AttacksTo(square, occupied);
}
#endif

}  // namespace chess

#endif  // CHESS_STATIC_EXCHANGE_H_
