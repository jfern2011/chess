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
 * Decrement a value by 1 if black
 *
 * @{
 */
template <Player P>
constexpr int DecrementIfBlack(int i) noexcept(P != Player::kBoth) {
    throw std::logic_error(__func__);
}
template <> constexpr int DecrementIfBlack<Player::kWhite>(int i) noexcept {
    return i;
}
template <> constexpr int DecrementIfBlack<Player::kBlack>(int i) noexcept {
    return i-1;
}
/**
 * @}
 */

/**
 * Extract bit-packed move information. Moves are packed into 21 bits:
 *
 * 20...18: promotion piece
 * 17...15: captured piece
 * 14...12: piece moved
 * 11... 6: destination square
 *  5... 0: origin square
 *
 * @{
 */
constexpr Piece ExtractCaptured(std::int32_t move) noexcept {
    return static_cast<Piece>((move >> 15) & 0x7);
}

constexpr Square ExtractFrom(std::int32_t move) noexcept {
    return static_cast<Square>(move & 0x3F);
}

constexpr Piece ExtractMoved(std::int32_t move) noexcept {
    return static_cast<Piece>((move >> 12) & 0x7);
}

constexpr Piece ExtractPromoted(std::int32_t move) noexcept {
    return static_cast<Piece>((move >> 18) & 0x7);
}

constexpr Square ExtractTo(std::int32_t move) noexcept {
    return static_cast<Square>((move >> 6)  & 0x3F);
}
/**
 * @}
 */

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
 * Increment a value by 1 if black
 *
 * @{
 */
template <Player P>
constexpr int IncrementIfBlack(int i) noexcept(P != Player::kBoth) {
    throw std::logic_error(__func__);
}
template <> constexpr int IncrementIfBlack<Player::kWhite>(int i) noexcept {
    return i;
}
template <> constexpr int IncrementIfBlack<Player::kBlack>(int i) noexcept {
    return i+1;
}
/**
 * @}
 */

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

constexpr std::int8_t Lsb(std::uint64_t qword) noexcept;
constexpr std::int8_t Msb(std::uint64_t qword) noexcept;

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

/**
 * Pack a move into its 21-bit encoded form
 *
 * @param[in] captured The piece captured
 * @param[in] from     The origin square
 * @param[in] moved    The piece that was moved
 * @param[in] promoted The piece promoted to
 * @param[in] to       The destination square
 *
 * @return The bit-packed move
 */
constexpr std::int32_t PackMove(int captured,
                                int from,
                                int moved,
                                int promoted,
                                int to) noexcept {
    return (captured << 15) |
           (from) |
           (moved << 12) |
           (promoted << 18) |
           (to << 6);
}

Piece  CharToPiece(char piece);
char   PieceToChar(Piece piece, bool to_lower = false);
Square StrToSquare(const std::string& str);

}  // namespace util
}  // namespace chess

#endif  // UTIL_H_
