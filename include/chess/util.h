/**
 *  \file   util.h
 *  \author Jason Fernandez
 *  \date   01/19/2020
 */

#ifndef UTIL_H_
#define UTIL_H_

#include <cstddef>
#include <cstdint>
#include <type_traits>
#include <vector>

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
 * Count the number of bits set in a word in O(n) time
 *
 * @param [in] word An n-bit word
 *
 * @return The number of bits set in the word
 */
template<typename T> constexpr std::uint8_t BitCount(T word) {
    std::uint8_t count = 0;

    for (; word; count++) word &= word - 1;

    return count;
}

/**
 * Clear the specified bit within a word
 *
 * @param [in]     bit  The bit to clear
 * @param [in,out] word An n-bit word
 */
template<typename T> constexpr void ClearBit(int bit, T* word) {
    *word ^= (*word & (T(1) << bit));
}

/**
 * Clear the specified bits of a word
 *
 * @param [in]     mask Specifies the bits within \a word to clear
 * @param [in,out] word An n-bit word
 */
template<typename T> constexpr void ClearBits(T mask, T* word) {
    *word ^= (*word & mask);
}

/**
 * Recursive base case of \ref CreateBitMask
 *
 * @tparam T The integral type of this mask
 *
 * @return 0
 */
template <typename T>
constexpr T CreateBitMask() {
    return T(0);
}

/**
 * Create a bitmask with the specified bits set
 *
 * @tparam T  The integral type of this mask
 * @tparam I1 A bit index to set to 1
 * @tparam Is Additional bit indexes to set to 1
 *
 * @return The desired bitmask
 */
template <typename T, int I1, int... Is>
constexpr T CreateBitMask() {
    return (T(1) << I1) | CreateBitMask<T,Is...>();
}

/**
 * Retrieve a bitmask with only the specified bit set
 *
 * @param [in] bit The desired bit, indexed from 0
 *
 * @return A power of 2 whose base-2 logarithm = \a bit. If \a bit exceeds the
 *         size of T (in bits), the result is undefined
 */
template<typename T> constexpr T GetBit(int bit) {
    return (T(1)) << bit;
}

/**
 * Retrieve a 64-bit mask with only 1 bit set to represent a square
 *
 * @param [in] square The desired square
 *
 * @return A power of 2 whose base-2 logarithm = \a square
 */
std::uint64_t constexpr GetBit(Square square) {
    return std::uint64_t(1) << square;
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
constexpr int GetFile(int square) {
    return square & 0x7;
}

/**
 * Get a bitmask representing the file containing a given square
 *
 * @param[in] square Get the file this square is on
 *
 * @return A bitmask with 1 bits for all squares along this file
 */
constexpr std::uint64_t GetFileMask(int square) {
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
 * Get the index of the least significant bit set in O(n) time
 *
 * @param [in] word The word to scan
 *
 * @return The LSB, or -1 if no bits are set
 */
template<typename T> constexpr std::int8_t GetLsb(T word) {
    std::int8_t bit = 0; T mask = 1;

    if (word != 0) {
        while (!(mask & word)) {
            mask <<= 1; bit += 1;
        }
        return bit;
    }

    return -1;
}

/**
 * Get the index of the most significant bit set in O(n) time
 *
 * @param [in] word The word to scan
 *
 * @return The MSB, or -1 if no bits are set
 */
template<typename T> constexpr std::int8_t GetMsb(T word) {
    std::int8_t bit = (8 * sizeof(T) - 1 );
    T mask = (T(1)) << bit;

    if (word != 0) {
        while (!(mask & word)) {
            mask >>= 1; bit -= 1;
        }
        return bit;
    }

    return -1;
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
constexpr int GetRank(int square) {
    return square >> 3;
}

/**
 * Get a bitmask representing the rank containing a given square
 *
 * @param[in] square Get the rank this square is on
 *
 * @return A bitmask with 1 bits for all squares along this rank
 */
constexpr std::uint64_t GetRankMask(int square) {
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
 *  Returns the indexes of all bits set in a word
 *
 * @param [in]  word    The word to parse
 * @param [out] indexes A list of bit indexes set
 *
 * @return The number of bits set
 */
template<typename T> constexpr
std::size_t GetSetBits(T word, int* indexes) {
    std::size_t count = 0;

    while (word) {
        const int lsb = GetLsb(word);
        indexes[count++] = lsb;

        ClearBit(lsb, &word);
    }

    return count;
}

/**
 *  Returns the indexes of all bits set in a word
 *
 * @param [in]  word    The word to parse
 * @param [out] indexes A list of bit indexes set
 *
 * @return True on success
 */
template<typename T>
bool GetSetBits(T word, std::vector<int>* indexes) {
    indexes->clear();

    while (word) {
        const int lsb = GetLsb(word);
        indexes->push_back(lsb);

        ClearBit(lsb, &word);
    }

    return true;
}

}  // namespace util
}  // namespace chess

#endif  // UTIL_H_
