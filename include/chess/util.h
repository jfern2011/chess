/**
 *  \file   util.h
 *  \author Jason Fernandez
 *  \date   01/19/2020
 */

#ifndef UTIL_H_
#define UTIL_H_

#include <cctype>
#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <type_traits>
#include <vector>

#include "bitops/bitops.h"
#include "chess/chess.h"

namespace chess {
namespace util {

/**
 * Convert an enumeration type to its underlying integral value
 *
 * @tparam E The enum type (scoped or unscoped)
 *
 * @param[in] value The enumerator value to convert
 *
 * @return The integer-equivalent of \a value
 */
template <typename E>
constexpr typename std::underlying_type_t<E>
ToIntType(E value) noexcept {
    return static_cast<std::underlying_type_t<E>>(value);
}

/**
 * Retrieve a 64-bit mask with only 1 bit set to represent a square
 *
 * @param [in] square The desired square
 *
 * @return A power of 2 whose base-2 logarithm = \a square
 */
constexpr std::uint64_t GetBit(Square square) noexcept {
    return jfern::bitops::get_bit<std::uint64_t>(ToIntType(square));
}

/**
 * Get the file of a particular square, indexed from zero
 *
 * @note The H-file corresponds to index 0
 *
 * @param[in] square The square
 *
 * @return The file that \a square is on
 */
constexpr int GetFile(int square) noexcept {
    return square & 0x7;
}

/**
 * Get a bitmask representing the file containing a given square
 *
 * @param[in] square Get the file this square is on
 *
 * @return A bitmask with 1 bits for all squares along this file
 */
constexpr std::uint64_t GetFileMask(int square) noexcept {
    constexpr std::uint64_t arr[64] = {
        kFileH, kFileG, kFileF, kFileE, kFileD, kFileC, kFileB, kFileA,
        kFileH, kFileG, kFileF, kFileE, kFileD, kFileC, kFileB, kFileA,
        kFileH, kFileG, kFileF, kFileE, kFileD, kFileC, kFileB, kFileA,
        kFileH, kFileG, kFileF, kFileE, kFileD, kFileC, kFileB, kFileA,
        kFileH, kFileG, kFileF, kFileE, kFileD, kFileC, kFileB, kFileA,
        kFileH, kFileG, kFileF, kFileE, kFileD, kFileC, kFileB, kFileA,
        kFileH, kFileG, kFileF, kFileE, kFileD, kFileC, kFileB, kFileA,
        kFileH, kFileG, kFileF, kFileE, kFileD, kFileC, kFileB, kFileA
    };

    return arr[square];
}

/**
 * Get the rank of a particular square, indexed from zero
 *
 * @note White's back rank corresponds to index 0
 *
 * @param[in] square The square
 *
 * @return The rank that \a square is on
 */
constexpr int GetRank(int square) noexcept {
    return square >> 3;
}

/**
 * Get a bitmask representing the rank containing a given square
 *
 * @param[in] square Get the rank this square is on
 *
 * @return A bitmask with 1 bits for all squares along this rank
 */
constexpr std::uint64_t GetRankMask(int square) noexcept {
    constexpr std::uint64_t arr[64] = {
        kRank1, kRank1, kRank1, kRank1, kRank1, kRank1, kRank1, kRank1,
        kRank2, kRank2, kRank2, kRank2, kRank2, kRank2, kRank2, kRank2,
        kRank3, kRank3, kRank3, kRank3, kRank3, kRank3, kRank3, kRank3,
        kRank4, kRank4, kRank4, kRank4, kRank4, kRank4, kRank4, kRank4,
        kRank5, kRank5, kRank5, kRank5, kRank5, kRank5, kRank5, kRank5,
        kRank6, kRank6, kRank6, kRank6, kRank6, kRank6, kRank6, kRank6,
        kRank7, kRank7, kRank7, kRank7, kRank7, kRank7, kRank7, kRank7,
        kRank8, kRank8, kRank8, kRank8, kRank8, kRank8, kRank8, kRank8
    };

    return arr[square];
}

/**
 * Get a consistent array index as a function of player
 *
 * @{
 */
template <Player P> constexpr int index() noexcept(P != Player::kBoth) {
    throw std::logic_error(__func__);
}
template <> constexpr int index<Player::kBlack>() noexcept {
    return 0;
}
template <> constexpr int index<Player::kWhite>() noexcept {
    return 1;
}
/**
 * @}
 */

/**
 * Get the opposing side
 *
 * @{
 */
template <Player P> constexpr Player opponent() noexcept(P != Player::kBoth) {
    throw std::logic_error(__func__);
}
template <> constexpr Player opponent<Player::kBlack>() noexcept {
    return Player::kWhite;
}
template <> constexpr Player opponent<Player::kWhite>() noexcept {
    return Player::kBlack;
}
/**
 * @}
 */

Piece  CharToPiece(char piece);
char   PieceToChar(Piece piece, bool to_lower = false);
Square StrToSquare(const std::string& str);

}  // namespace util
}  // namespace chess

#endif  // UTIL_H_
