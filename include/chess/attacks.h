/**
 *  \file   attacks.h
 *  \author Jason Fernandez
 *  \date   07/03/2020
 */

#ifndef CHESS_ATTACKS_H_
#define CHESS_ATTACKS_H_

#include "chess/chess.h"
#include "chess/data_tables.h"

namespace chess {

/**
 * "Attacks from" bitboard generators for sliding pieces
 * @{
 */

/**
 * @tparam piece The attacking piece
 *
 * @param[in] square   The square from which this piece is attacking
 * @param[in] occupied The occupied squares bitboard
 *
 * @return The set of squares attacked by the specified piece
 */
template <Piece piece>
std::uint64_t AttacksFrom(Square square, std::uint64_t occupied) {
    return 0;
}

template <>
constexpr std::uint64_t AttacksFrom<Piece::BISHOP>(Square square,
                                                   std::uint64_t occupied) {
    const std::uint64_t occupied_ =
        data_tables::kBishopAttacksMask[square] & occupied;

    const std::uint32_t index =
        chess::data_tables::kBishopOffsets[square] +
            ((occupied_ * chess::data_tables::kDiagMagics[square]) >>
                chess::data_tables::kBishopDbShifts[square]);

    return chess::data_tables::kBishopAttacks[index];
}

template <>
constexpr std::uint64_t AttacksFrom<Piece::ROOK>(Square square,
                                                 std::uint64_t occupied) {
    const std::uint64_t occupied_ =
        data_tables::kRookAttacksMask[square] & occupied;

    const std::uint32_t index =
        chess::data_tables::kRookOffsets[square] +
            ((occupied_ * chess::data_tables::kRookMagics[square]) >>
                chess::data_tables::kRookDbShifts[square]);

    return chess::data_tables::kRookAttacks[index];
}

template <>
constexpr std::uint64_t AttacksFrom<Piece::QUEEN>(Square square,
                                                  std::uint64_t occupied) {
    return AttacksFrom<Piece::BISHOP>(square, occupied) |
           AttacksFrom<Piece::ROOK  >(square, occupied);
}
/**
 * @}
 */

}  // namespace chess

#endif  // CHESS_ATTACKS_H_
