/**
 *  \file   data_tables_internal.h
 *  \author Jason Fernandez
 *  \date   04/11/2020
 */

#ifndef DATA_TABLES_INTERNAL_H_
#define DATA_TABLES_INTERNAL_H_

#include <array>
#include <cstddef>
#include <cstdint>
#include <utility>

#include "chess/chess.h"
#include "chess/util.h"

namespace chess {
namespace data_tables {
namespace internal {

/** The number of slots in the "attacks from" database for bishops */
constexpr std::uint32_t kAttacksDiagDbSize = 0x01480;

/** The number of slots in the "attacks from" database for rooks   */
constexpr std::uint32_t kAttacksRookDbSize = 0x19000;

/**
 * The set of all ways a file/rank pair or pair of diagonals may be occupied,
 * excluding the edges and the square over which they cross. For example, the
 * occupancy set for E4 includes the B1-H7 and H1-A8 diagonals, except for
 * B1, H7, H1, A8, and E4. That leaves us with 9 squares, so the occupancy
 * set consists of all 512 ways these squares may be occupied. If on the other
 * hand we're interested in file/rank pairs, the occupancy set for E4 would
 * include all ways for 10 squares to be occupied, that is 1024
 *
 * @tparam N The maximum size of the occupancy set
 */
template <std::size_t N> struct OccupancySet {
    std::size_t size;        /*!< Number of slots used   */
    std::uint64_t table[N];  /*!< The set of occupancies */
};

/**
 * Create a compile-time constant table row
 *
 * @tparam F  A function object that initializes each element of the table
 *            as a function of its indexes
 * @tparam R  A parameter pack of size 0 or 1. The function object \a F takes
 *            either 1 or 2 arguments to compute an element within a 1 or 2-
 *            dimensional table, respectively. For 2-dimensional tables, this
 *            parameter is the table row. For 1-dimensional tables, this is
 *            an empty parameter pack
 * @tparam C  A compile-time sequence of integers, whose length is the number
 *            of columns in the row
 *
 * @param[in] func  Computes the initial value of each table entry
 *
 * @return A new table row
  */
template <class F, std::size_t... R, std::size_t... C>
constexpr auto MakeArray(F&& func, std::index_sequence<C...>) {
    return std::array<decltype(func(R..., 0)), sizeof...(C)>{func(R..., C)...};
}

/**
 * Create a compile-time constant 2-dimensional table
 *
 * @tparam F  A function object that initializes each element of the table
 *            as a function of its indexes
 * @tparam Is A compile-time sequence of integers, whose length determines the
 *            number of rows in the table
 * @tparam Js A compile-time sequence of integers, whose length determines the
 *            number of columns in the table
 *
 * @param[in] func  Computes the initial value of each table entry
 *
 * @return A new 2-dimensional table
  */
template <class F, std::size_t... Is, std::size_t... Js>
constexpr auto MakeArray(F&& func, std::index_sequence<Is...>,
                                   std::index_sequence<Js...>) {
    constexpr auto NJs = sizeof...(Js);
    return
        std::array<std::array<decltype(func(0, 0)), NJs>,
            sizeof...(Is)>{
                MakeArray<F,Is>(func, std::make_index_sequence<NJs>())...
        };
}

/**
 * Create a compile-time constant 1 or 2-dimensional table
 *
 * @tparam Is A compile-time sequence of integers, whose length determines the
 *            dimensions of the table. Elements are in row-major order
 * @tparam F  A function object that initializes every element of the table as
 *            a function of its indexes
 *
 * @param[in] func  Computes the initial value of each table entry
 *
 * @return A new constant table
  */
template <std::size_t... I, class F>
constexpr auto CreateTable(F&& func) {
    return MakeArray(func, std::make_index_sequence<I>()...);
}

/**
 * Get a 64-bit bitmask that has all bits set except at the specified index
 *
 * @param [in] square The bit index
 *
 * @return A bitmask with all bits set except at index \a square
 */
constexpr std::uint64_t ClearMask(int square) {
    return ~(std::uint64_t(1) << square);
}

/**
 * En-passant target squares. These are invalid except for the 4th and 5th
 * ranks. For example, if a pawn advanced to E4, then the target square is E3
 *
 * @param[in] square The square reached by a pawn that advanced twice
 *
 * @return The target square
 */
constexpr Square EpTarget(int square) {
    switch (util::GetRank(square)) {
        case 3: return static_cast<Square>(square-8);
        case 4: return static_cast<Square>(square+8);
        default:
            return Square::Underflow;
    }
}

/**
 * Get the diagonal that the given square is on in the A1-H8 direction
 *
 * @param [in] square The square
 *
 * @return The diagonal containing this square
 */
constexpr std::uint64_t GetDiagA1H8(int square) {
    std::uint64_t one = 1, diag = 0;

    for (int sq = square; sq < 64; sq += 7) {
        diag |= one << sq;
        if (util::GetFile(sq) == 0) break;
    }

    for (int sq = square; sq >= 0; sq -= 7) {
        diag |= one << sq;
        if (util::GetFile(sq) == 7) break;
    }

    return diag;
}

/**
 * Get the diagonal that the given square is on in the H1-A8 direction
 *
 * @param [in] square The square
 *
 * @return The diagonal containing this square
 */
constexpr std::uint64_t GetDiagH1A8(int square) {
    std::uint64_t one = 1, diag = 0;

    for (int sq = square; sq < 64; sq += 9) {
        diag |= one << sq;
        if (util::GetFile(sq) == 7) break;
    }

    for (int sq = square; sq >= 0; sq -= 9) {
        diag |= one << sq;
        if (util::GetFile(sq) == 0) break;
    }

    return diag;
}

/**
 * Get the direction along which two squares lie
 *
 * @param[in] square1 The 1st square
 * @param[in] square2 The 2nd square
 *
 * @return The direction shared by these squares
 */
constexpr Direction GetDirection(int square1, int square2) {
    if (square1 == square2)
        return Direction::kNone;

    if (GetDiagH1A8(square1) == GetDiagH1A8(square2))
        return Direction::kAlongH1A8;

    if (util::GetRank(square1) == util::GetRank(square2))
        return Direction::kAlongRank;

    if (GetDiagA1H8(square1) == GetDiagA1H8(square2))
        return Direction::kAlongA1H8;

    if (util::GetFile(square1) == util::GetFile(square2))
        return Direction::kAlongFile;

    return Direction::kNone;
}

/**
 * Get a bishop range mask, which is the set of all squares reachable by a
 * bishop, including the square itself, if that bishop is not obstructed in any
 * direction
 *
 * @param[in] square Get the bitmask for a bishop on this square
 *
 * @return The range mask
 */
constexpr std::uint64_t BishopRangeMask(int square) {
    return GetDiagA1H8(square) | GetDiagH1A8(square);
}

/**
 * Get all squares "northeast" of a particular square, when viewed from
 * White's perpective
 *
 * @param[in] square Get all squares northeast of this
 *
 * @return The northeast bitmask
 */
constexpr std::uint64_t NorthEastMask(int square) {
    std::uint64_t mask = 0;

    for (int i = square; i < 64
         && (GetDiagA1H8(i) & (std::uint64_t(1) << i)); i += 7) {
        if (i != square) mask |= std::uint64_t(1) << i;
        if (util::GetFile(i) == 0) break;
    }

    return mask;
}

/**
 * Get all squares "northwest" of a particular square, when viewed from
 * White's perpective
 *
 * @param[in] square Get all squares northwest of this
 *
 * @return The northwest bitmask
 */
constexpr std::uint64_t NorthWestMask(int square) {
    std::uint64_t mask = 0;

    for (int i = square; i < 64
         && (GetDiagH1A8(i) & (std::uint64_t(1) << i)); i += 9) {
        if (i != square) mask |= std::uint64_t(1) << i;
        if (util::GetFile(i) == 7) break;
    }

    return mask;
}

/**
 * Get all squares "southeast" of a particular square, when viewed from
 * White's perpective
 *
 * @param[in] square Get all squares southeast of this
 *
 * @return The southeast bitmask
 */
constexpr std::uint64_t SouthEastMask(int square) {
    std::uint64_t mask = 0;

    for (int i = square; i >= 0
         && (GetDiagH1A8(i) & (std::uint64_t(1) << i)); i -= 9) {
        if (i != square) mask |= std::uint64_t(1) << i;
        if (util::GetFile(i) == 0) break;
    }

    return mask;
}

/**
 * Get all squares "southwest" of a particular square, when viewed from
 * White's perpective
 *
 * @param[in] square Get all squares southwest of this
 *
 * @return The southwest bitmask
 */
constexpr std::uint64_t SouthWestMask(int square) {
    std::uint64_t mask = 0;

    for (int i = square; i >= 0
         && (GetDiagA1H8(i) & (std::uint64_t(1) << i)); i -= 7) {
        if (i != square) mask |= std::uint64_t(1) << i;
        if (util::GetFile(i) == 7) break;
    }

    return mask;
}

/**
 * Given an occupancy bitboard, compute the squares attacked by a bishop
 *
 * @param [in] square   The square the bishop is on
 * @param [in] occupied The squares occupied by all other pieces
 *
 * @return A bitboard with 1 bits for all the squares attacked by this bishop
 */
constexpr std::uint64_t AttacksFromDiag(int square,
                                        std::uint64_t occupied) {
    std::uint64_t attacks =
        BishopRangeMask(square) ^ util::GetBit<std::uint64_t>(square);

    int blocker =
        util::GetLsb(occupied & NorthEastMask(square));

    if (blocker != -1)
        attacks ^= NorthEastMask(blocker);

    blocker =
        util::GetMsb(occupied & SouthEastMask(square));

    if (blocker != -1)
        attacks ^= SouthEastMask(blocker);

    blocker =
        util::GetLsb(occupied & NorthWestMask(square));

    if (blocker != -1)
        attacks ^= NorthWestMask(blocker);

    blocker =
        util::GetMsb(occupied & SouthWestMask(square));

    if (blocker != -1)
        attacks ^= SouthWestMask(blocker);

    return attacks;
}

/**
 * Get a rook range mask, which is the set of all squares reachable by a rook,
 * including the square itself, if that rook is not obstructed in any direction
 *
 * @param[in] square Get the bitmask for a rook on this square
 *
 * @return The range mask
 */
constexpr std::uint64_t RookRangeMask(int square) {
    return util::GetFileMask(square) | util::GetRankMask(square);
}

/**
 * Get all squares "north" of a particular square, when viewed from White's
 * perpective
 *
 * @param[in] square Get all squares north of this
 *
 * @return The north bitmask
 */
constexpr std::uint64_t NorthMask(int square) {
    std::uint64_t mask = 0;

    for (int i = square + 8; i < 64; i += 8)
        mask |= std::uint64_t(1) << i;

    return mask;
}

/**
 * Get all squares "south" of a particular square, when viewed from White's
 * perpective
 *
 * @param[in] square Get all squares south of this
 *
 * @return The south bitmask
 */
constexpr std::uint64_t SouthMask(int square) {
    std::uint64_t mask = 0;

    for (int i = square - 8; i >= 0; i -= 8)
        mask |= std::uint64_t(1) << i;

    return mask;
}

/**
 * Get all squares "east" of a particular square, when viewed from White's
 * perpective
 *
 * @param[in] square Get all squares east of this
 *
 * @return The east bitmask
 */
constexpr std::uint64_t EastMask(int square) {
    std::uint64_t mask = 0;

    for (int i = square - 1;
         i >= 0 && util::GetRank(i) == util::GetRank(square); i--)
        mask |= std::uint64_t(1) << i;

    return mask;
}

/**
 * Get all squares "west" of a particular square, when viewed from White's
 * perpective
 *
 * @param[in] square Get all squares west of this
 *
 * @return The west bitmask
 */
constexpr std::uint64_t WestMask(int square) {
    std::uint64_t mask = 0;

    for (int i = square + 1;
         i < 64 && util::GetRank(i) == util::GetRank(square); i++)
        mask |= std::uint64_t(1) << i;

    return mask;
}

/**
 * Given an occupancy bitboard, computes the squares attacked by a rook
 *
 * @param [in] square   The square the rook is on
 * @param [in] occupied The squares occupied by all other pieces
 *
 * @return A bitboard with bits set for all the squares attacked by this rook
 */
constexpr std::uint64_t AttacksFromRook(int square,
                                        std::uint64_t occupied) {
    std::uint64_t attacks =
        RookRangeMask(square) ^ util::GetBit< std::uint64_t >(square);

    int blocker =
        util::GetLsb(occupied & NorthMask(square));

    if (blocker != -1)
        attacks ^= NorthMask(blocker);

    blocker =
        util::GetLsb(occupied & WestMask(square));

    if (blocker != -1)
        attacks ^= WestMask(blocker);

    blocker =
        util::GetMsb(occupied & EastMask(square));

    if (blocker != -1)
        attacks ^= EastMask(blocker);

    blocker =
        util::GetMsb(occupied & SouthMask(square));

    if (blocker != -1)
        attacks ^= SouthMask(blocker);

    return attacks;
}

/**
 * Get a bishop occupancy mask, which we bitwise AND with the occupied squares
 * board in the magic bitboard move generation algorithm
 *
 * @param[in] square The square for which to get the occupancy mask
 *
 * @return The occupancy mask
 */
constexpr std::uint64_t BishopOccupancyMask(int square) {
    const std::uint64_t frame = kRank1 | kRank8 | kFileA | kFileH;

    std::uint64_t scope = GetDiagA1H8(square) | GetDiagH1A8(square);

    scope ^= scope & (frame | util::GetBit<std::uint64_t>(square));

    return scope;
}

/**
 * Get a rook occupancy mask, which we bitwise AND with the occupied squares
 * board in the magic bitboard move generation algorithm
 *
 * @param[in] square The square for which to get the occupancy mask
 *
 * @return The occupancy mask
 */
constexpr std::uint64_t RookOccupancyMask(int square) {
    std::uint64_t frame = kRank1 | kRank8 | kFileA | kFileH;

    std::uint64_t scope = util::GetFileMask(square) | util::GetRankMask(square);

    if (util::GetFile(square) == 0) {
        frame ^= kFileH ^ util::GetBit(Square::H1) ^ util::GetBit(Square::H8);
    } else if (util::GetFile(square) == 7) {
        frame ^= kFileA ^ util::GetBit(Square::A1) ^ util::GetBit(Square::A8);
    }

    if (util::GetRank(square) == 0) {
        frame ^= kRank1 ^ util::GetBit(Square::A1) ^ util::GetBit(Square::H1);
    } else if (util::GetRank(square) == 7) {
        frame ^= kRank8 ^ util::GetBit(Square::A8) ^ util::GetBit(Square::H8);
    }

    scope ^= scope & (frame | (std::uint64_t(1) << square));

    return scope;
}

/**
 * Get the size of the bit shift to the right required by the magic bitboard
 * move generation algorithm to obtain an index into the bishop "attacks from"
 * database
 *
 * @param [in] square Each square requires a different shift
 *
 * @return The size of the bit shift
 */
constexpr int BishopDbShift(int square) {
    return 64 - util::BitCount(BishopOccupancyMask(square));
}

/**
 * Get the size of the bit shift to the right required by the magic bitboard
 * move generator to obtain an index into the rook "attacks from" database
 *
 * @param [in] square Each square requires a different shift
 *
 * @return The size of the bit shift
 */
constexpr int RookDbShift(int square) {
    return 64 - util::BitCount(RookOccupancyMask(square));
}

/**
 * Get the offset at which the bishop "attacks from" database begins for a
 * particular square
 *
 * @param[in] square The square for which to get the offset
 *
 * @return The "attacks from" table offset
 */
constexpr std::uint32_t DiagOffset(int square) {
    std::uint32_t prev_offset = 0;
    
    for (int i = 1; i <= square; i++)
        prev_offset +=
            std::uint64_t(1) << util::BitCount(BishopOccupancyMask(i-1));

    return prev_offset;
}

/**
 * Get the offset at which the rook "attacks from" database begins for a
 * particular square
 *
 * @param[in] square The square for which to get the offset
 *
 * @return The "attacks from" table offset
 */
constexpr std::uint32_t RookOffset(int square) {
    std::uint32_t prev_offset = 0;
    
    for (int i = 1; i <= square; i++)
        prev_offset +=
            std::uint64_t(1) << util::BitCount(RookOccupancyMask(i-1));

    return prev_offset;
}

/**
 * Returns the indexes of all bits set in a word
 *
 * @note Used as a fast compile-time alternative to \ref util::GetSetBits()
 *
 * @param [in]  word    The word to parse
 * @param [out] indexes A list of bit indexes set
 *
 * @return The number of bits set
 */
template<typename T>
constexpr std::size_t GetSetBits(T word, int* indexes) {
    std::size_t count = 0;

    while (word) {
        const std::int8_t lsb = util::GetLsb<T>(word);
        util::ClearBit( lsb, &word );
        indexes[count++] = lsb;
    }

    return count;
}

/**
 * @brief
 * Generate an occupancy set (collection of bitboards) for a bishop on
 * the given square
 *
 * @details
 * An "occupancy set" is the set of all the occupancy bitmasks that would
 * affect the range of squares a bishop on \a square could attack. For
 * example, the squares attacked by a bishop on E4 are determined by the
 * occupancies of the H1-A8 and B1-H7 diagonals. Because as many as 9
 * squares may be occupied (excluding edges of the board and the square the
 * bishop is on), there are a total of 2^9 occupancy masks in the set
 *
 * @param[in]  square The square to create the occupancy set for
 *
 * @return The occupancy set
 */
constexpr OccupancySet<512> GenDiagOccupancies(int square) {
    OccupancySet<512> set = {0, {0}};

    std::uint64_t diags = BishopOccupancyMask(square);

    std::uint64_t bit_masks[64] = {};

    int set_bits[64] = {};
    const std::size_t nbits = GetSetBits(diags, set_bits);

    for (std::size_t i = 0; i < nbits; i++) {
        bit_masks[i] =
            util::GetBit< std::uint64_t >(set_bits[i]);
    }

    for (int i = 0; i < (1 << nbits); i++) {
        int i_bits[64] = {};
        const std::size_t n_ibits = GetSetBits(i, i_bits);
        std::uint64_t mask = 0;

        for (std::size_t j = 0; j < n_ibits; j++) {
            mask |= bit_masks[i_bits[j]];
        }

        set.table[set.size++] = mask;
    }

    return set;
}

/**
 * @brief
 * Generate the occupancy set (collection of bitboards) for a rook on the
 * given square
 *
 * @details
 * An "occupancy set" is the set of all the occupancy bitmasks that would
 * affect the range of squares a rook on \a square could attack. For example,
 * the squares attacked by a rook on E4 are determined by the occupancies of
 * the 4th rank and E-file. Because as many as 10 squares may be occupied
 * (excluding the edges of the board and the square the rook is on) there are
 * a total of of 2^10 occupancy masks in the set
 *
 * @param[in]  square The square to create the occupancy set for
 *
 * @return The occupancy set
 */
constexpr OccupancySet<4096> GenRookOccupancies(int square) {
    OccupancySet<4096> set = {0, {0}};

    std::uint64_t range = RookOccupancyMask(square);

    std::uint64_t bit_masks[64] = {};

    int set_bits[64] = {};
    const std::size_t nbits = util::GetSetBits(range, set_bits);

    for (std::size_t i = 0; i < nbits; i++) {
        bit_masks[i] =
            util::GetBit< std::uint64_t >(set_bits[i]);
    }

    for (int i = 0; i < (1 << nbits); i++) {
        int i_bits[64] = {};
        const std::size_t n_ibits = util::GetSetBits(i, i_bits);
        std::uint64_t mask = 0;

        for (std::size_t j = 0; j < n_ibits; j++) {
            mask |= bit_masks[i_bits[j]];
        }

        set.table[set.size++] = mask;
    }

    return set;
}

/**
 * Get the "magic" number used to look up an "attacks from" bitboard for
 * a bishop on a particular square
 *
 * @note I totally lost the code that originally computed suitable magic
 *       numbers. These values are copied from its output
 *
 * @param[in] square Get the magic number for this square
 *
 * @return The magic number
 */
constexpr std::uint64_t DiagMagic(int square) {
    constexpr std::uint64_t arr[64] = {
        0x03044810010A08B0ULL, 0x2090010101220004ULL,
        0x4008128112080140ULL, 0x0049040309204160ULL, 
        0x2004046020020418ULL, 0x5043012010001020ULL, 
        0x0004044148080000ULL, 0x1000410828030402ULL,
        0x0000300A08080085ULL, 0x0030102200840290ULL, 
        0x0000041400820020ULL, 0x0008782049400000ULL,
        0x6009020210000060ULL, 0x4000010420050000ULL, 
        0x000102820510400CULL, 0x28200201441C4420ULL,
        0x0008181142484800ULL, 0x0850040204014408ULL, 
        0x0030020805202024ULL, 0x0022000403220120ULL,
        0x0312008401A21820ULL, 0x1002011409820820ULL, 
        0x0122040100822008ULL, 0x0841084140425008ULL,
        0x0020200008130C01ULL, 0x0042100020010210ULL, 
        0x0044021001080900ULL, 0x1004010006490100ULL,
        0x1040404004010043ULL, 0x10480A0040220100ULL, 
        0x400802C062024200ULL, 0x0200404001840400ULL,
        0x2401080841405180ULL, 0x2008010404904428ULL, 
        0x0010404800900220ULL, 0x0241010802010040ULL,
        0x00304C0400004100ULL, 0x0030084200044100ULL, 
        0x1102408A00011801ULL, 0x080801044281004AULL,
        0x0001080840004428ULL, 0x0211241024040210ULL, 
        0x0280420045003001ULL, 0x1000006011080800ULL,
        0x1014022039000200ULL, 0x4810013001881B00ULL, 
        0x20E8020808582210ULL, 0x0808010C28804828ULL,
        0x14841404040E2404ULL, 0x0081221910480400ULL, 
        0x2080010401040000ULL, 0x0020080242022102ULL,
        0x1000001012020200ULL, 0x4050223401120002ULL, 
        0x2024040448120400ULL, 0x4104413204090000ULL,
        0x0006004062103040ULL, 0x48000C420804220CULL, 
        0x0020005D08A80400ULL, 0x4020040150940404ULL,
        0x40C0000052160208ULL, 0x1800000408100108ULL, 
        0x002060202A0201C0ULL, 0x000C110204040081ULL
    };

    return arr[square];
}

/**
 * Get the "magic" number used to look up an "attacks from" bitboard for
 * a rook on a particular square
 *
 * @note I totally lost the code that originally computed suitable magic
 *       numbers. These values are copied from its output
 *
 * @param[in] square Get the magic number for this square
 *
 * @return The magic number
 */
constexpr std::uint64_t RookMagic(int square) {
    constexpr std::uint64_t arr[64] = {
        0x1880003023804000ULL, 0x4D40002001100040ULL, 
        0x0180181000802000ULL, 0x01000A1001002014ULL,
        0x020028A200041020ULL, 0x060008010A001004ULL, 
        0x1080020000800500ULL, 0x0200008204002841ULL,
        0x0013002040800304ULL, 0x0008400120005000ULL, 
        0x0001004020001301ULL, 0x0089002408100100ULL,
        0x0041001100180004ULL, 0x0041002604010018ULL, 
        0x10040018210A0410ULL, 0x1021000100006092ULL,
        0x0010608001824000ULL, 0x00C0008040200080ULL, 
        0x1139010044200011ULL, 0x0400210008100100ULL,
        0x4181030010080084ULL, 0x408400800CC20080ULL, 
        0x0018040068102102ULL, 0x1004020004204095ULL,
        0x1002008200250040ULL, 0x20100C4140012000ULL, 
        0x4103014100302000ULL, 0x2422001A00102040ULL,
        0x4000049100080100ULL, 0x2012005200110804ULL, 
        0x0041120400013008ULL, 0x0821002100004082ULL,
        0x00800420004002C0ULL, 0x0000200041401004ULL, 
        0x0000600501004090ULL, 0x0410002800801085ULL,
        0x011801004900100CULL, 0x0002000802000490ULL, 
        0x2F20021014000801ULL, 0x0008018402000043ULL,
        0x0080002002444000ULL, 0x2010002002404016ULL, 
        0x2005012000410010ULL, 0x0890003100190022ULL,
        0x0600050008010010ULL, 0x0104001008020200ULL, 
        0x2002020108240010ULL, 0x00025051208A0004ULL,
        0x0242010040802200ULL, 0x0000201002400240ULL, 
        0x4008590040200100ULL, 0x00400A2100100100ULL,
        0x0084280005001100ULL, 0x4001004802040100ULL, 
        0x6001004402000700ULL, 0x22000C884D140200ULL,
        0x0A80008020485103ULL, 0x0015108420400101ULL, 
        0x5080102000090041ULL, 0x0204211000080501ULL,
        0x4102002518102022ULL, 0x2401008804000201ULL, 
        0x4000020110080484ULL, 0x0000109040210402ULL
    };

    return arr[square];
}

/**
 * Get the bishop "attacks from" bitboard at the given table index
 *
 * @param[in] index The index computed by the magic bitboard hashing scheme
 *
 * @return The set of squares attacked by a bishop
 */
constexpr std::uint64_t InitAttacksFromDiag(std::uint32_t index) {

    // 1. Determine the square of the attacking bishop

    int from = 63;
    for (int i = 1; i < 64; i++) {
        if (DiagOffset(i) > index) {
            from = i-1; break;
        }
    }

    // 2. Find an occupancy board for which the hashing algorithm would yield
    //    the index passed to us

    const OccupancySet<512> set = GenDiagOccupancies(from);

    for (std::size_t i = 0; i < set.size; i++) {
        const std::uint32_t ind = DiagOffset(from) +
            ((DiagMagic(from) * set.table[i]) >> BishopDbShift(from));

        if (ind == index) return AttacksFromDiag(
            from, set.table[i]);
    }

    // This index is not used

    return 0;
}

/**
 * Get the rook "attacks from" bitboard at the given table index
 *
 * @param[in] index The index computed by the magic bitboard hashing scheme
 *
 * @return The set of squares attacked by a rook
 */
constexpr std::uint64_t InitAttacksFromRook(std::uint32_t index) {

    // 1. Determine the square of the attacking rook

    int from = 63;
    for (int i = 1; i < 64; i++) {
        if (RookOffset(i) > index) {
            from = i-1; break;
        }
    }

    // 2. Find an occupancy board for which the hashing algorithm would yield
    //    the index passed to us

    const OccupancySet<4096> set = GenRookOccupancies(from);

    for (std::size_t i = 0; i < set.size; i++) {
        const std::uint32_t ind = RookOffset(from) +
            ((RookMagic(from) * set.table[i]) >> RookDbShift(from));

        if (ind == index) return AttacksFromRook(
            from, set.table[i]);
    }

    // This index is not used

    return 0;
}

/**
 * Get the king "attacks from" bitboard at the given table index
 *
 * @param[in] square The square for which to generate the attacked squares
 *
 * @return The set of squares attacked by a king
 */
constexpr std::uint64_t InitAttacksFromKing(int square) {
    std::uint64_t attacks = 0;

    if (util::GetFile(square) < 7) {
        attacks |= util::GetBit<std::uint64_t>(square+1);
        if (util::GetRank(square) < 7)
            attacks |= util::GetBit<std::uint64_t>(square+9);
        if (util::GetRank(square) > 0)
            attacks |= util::GetBit<std::uint64_t>(square-7);
    }

    if (util::GetRank(square) < 7)
        attacks |= util::GetBit<std::uint64_t>(square+8);
    if (util::GetRank(square) > 0)
        attacks |= util::GetBit<std::uint64_t>(square-8);

    if (util::GetFile(square) > 0) {
        attacks |= util::GetBit<std::uint64_t>(square-1);
        if (util::GetRank(square) > 0)
            attacks |= util::GetBit<std::uint64_t>(square-9);
        if (util::GetRank(square) < 7)
            attacks |= util::GetBit<std::uint64_t>(square+7);
    }

    return attacks;
}

/**
 * Get the knight "attacks from" bitboard at the given table index
 *
 * @param[in] square The square for which to generate the attacked squares
 *
 * @return The set of squares attacked by a knight
 */
constexpr std::uint64_t InitAttacksFromKnight(int square) {
    std::uint64_t attacks = 0;

    if (util::GetFile(square) < 7) {
        if (util::GetRank(square) < 6)
            attacks |= util::GetBit<std::uint64_t>(
                square+17);
        if (util::GetRank(square) > 1)
            attacks |= util::GetBit<std::uint64_t>(
                square-15);
    }

    if (util::GetFile(square) < 6) {
        if (util::GetRank(square) < 7)
            attacks |= util::GetBit<std::uint64_t>(
                square+10);
        if (util::GetRank(square) > 0)
            attacks |= util::GetBit<std::uint64_t>(
                square- 6);
    }

    if (util::GetFile(square) > 0) {
        if (util::GetRank(square) < 6)
            attacks |= util::GetBit<std::uint64_t>(
                square+15);
        if (util::GetRank(square) > 1)
            attacks |= util::GetBit<std::uint64_t>(
                square-17);
    }

    if (util::GetFile(square) > 1) {
        if (util::GetRank(square) < 7)
            attacks |= util::GetBit<std::uint64_t>(
                square+ 6);
        if (util::GetRank(square) > 0)
            attacks |= util::GetBit<std::uint64_t>(
                square-10);
    }

    return attacks;
}

/**
 * Helper function used to ensure a square index is in bounds
 *
 * @param[in] square The square to check
 *
 * @return \a square, or \ref Overflow or \ref Underflow if the square is out
 *         of bounds
 */
constexpr Square InRange(int square) {
    if (square < 0 ) return Square::Underflow;
    if (square > 63) return Square::Overflow;

    return static_cast<Square>(square);
};

/**
 * Get the square arrived at by retreating 2 pawn steps from the given square
 *
 * @{
 */

template <Player P>
constexpr Square InitMinus16(int square) {
    return Square::Underflow;
}

template <>
constexpr Square InitMinus16<Player::kWhite>(int square) {
    return InRange(square-16);
}

template <>
constexpr Square InitMinus16<Player::kBlack>(int square) {
    return InRange(square+16);
}

/**
 * @}
 */

/**
 * Get the square arrived at by undoing a pawn capture to the right
 *
 * @{
 */

template <Player P>
constexpr Square InitMinus7(int square) {
    return Square::Underflow;
}

template <>
constexpr Square InitMinus7<Player::kWhite>(int square) {
    return util::GetFile(square) < 7 ? InRange(square-7) : Square::Underflow;
}

template <>
constexpr Square InitMinus7<Player::kBlack>(int square) {
    return util::GetFile(square) > 0 ? InRange(square+7) : Square::Underflow;
}

/**
 * @}
 */

/**
 * Get the square arrived at by retreating 1 pawn step from the given square
 *
 * @{
 */

template <Player P>
constexpr Square InitMinus8(int square) {
    return Square::Underflow;
}

template <>
constexpr Square InitMinus8<Player::kWhite>(int square) {
    return InRange(square-8);
}

template <>
constexpr Square InitMinus8<Player::kBlack>(int square) {
    return InRange(square+8);
}

/**
 * @}
 */

/**
 * Get the square arrived at by undoing a pawn capture to the left
 *
 * @{
 */

template <Player P>
constexpr Square InitMinus9(int square) {
    return Square::Underflow;
}

template <>
constexpr Square InitMinus9<Player::kWhite>(int square) {
    return util::GetFile(square) > 0 ? InRange(square-9) : Square::Underflow;
}

template <>
constexpr Square InitMinus9<Player::kBlack>(int square) {
    return util::GetFile(square) < 7 ? InRange(square+9) : Square::Underflow;
}

/**
 * @}
 */

/**
 * Get the square that a pawn can advance to from a given square
 *
 * @{
 */

template <Player P>
constexpr std::uint64_t InitPawnAdvances(int square) {
    return 0;
}

template <>
constexpr std::uint64_t InitPawnAdvances<Player::kWhite>(int square) {
    if (square < 56)
        return std::uint64_t(1) << InRange(square+8);
    return 0;
}

template <>
constexpr std::uint64_t InitPawnAdvances<Player::kBlack>(int square) {
    if (square > 7)
        return std::uint64_t(1) << InRange(square-8);
    return 0;
}

/**
 * Get the square(s) attacked by a pawn from a particular square
 *
 * @{
 */

template <Player P>
constexpr std::uint64_t InitPawnAttacks(int square) {
    return 0;
}

template <>
constexpr std::uint64_t InitPawnAttacks<Player::kWhite>(int square) {
    std::uint64_t attacks = 0;

    if (util::GetFile(square) < 7 && square < 55) {
        attacks |= std::uint64_t(1) << (square+9);
    }

    if (util::GetFile(square) > 0 && square < 56) {
        attacks |= std::uint64_t(1) << (square+7);
    }

    return attacks;
}

template <>
constexpr std::uint64_t InitPawnAttacks<Player::kBlack>(int square) {
    std::uint64_t attacks = 0;

    if (util::GetFile(square) < 7 && square > 7) {
        attacks |= std::uint64_t(1) << (square - 7);
    }

    if (util::GetFile(square) > 0 && square > 8) {
        attacks |= std::uint64_t(1) << (square - 9);
    }

    return attacks;
}

/**
 * @}
 */

/**
 * Get the square arrived at by advancing 2 pawn steps from the given square
 *
 * @{
 */

template <Player P>
constexpr Square InitPlus16(int square) {
    return Square::Underflow;
}

template <>
constexpr Square InitPlus16<Player::kWhite>(int square) {
    return InRange(square+16);
}

template <>
constexpr Square InitPlus16<Player::kBlack>(int square) {
    return InRange(square-16);
}

/**
 * @}
 */

/**
 * Get the square arrived at by making a pawn capture to the right
 *
 * @{
 */

template <Player P>
constexpr Square InitPlus7(int square) {
    return Square::Underflow;
}

template <>
constexpr Square InitPlus7<Player::kWhite>(int square) {
    return util::GetFile(square) > 0 ? InRange(square+7) : Square::Underflow;
}

template <>
constexpr Square InitPlus7<Player::kBlack>(int square) {
    return util::GetFile(square) < 7 ? InRange(square-7) : Square::Underflow;
}

/**
 * @}
 */

/**
 * Get the square arrived at by advancing 1 pawn step from the given square
 *
 * @{
 */

template <Player P>
constexpr Square InitPlus8(int square) {
    return Square::Underflow;
}

template <>
constexpr Square InitPlus8<Player::kWhite>(int square) {
    return InRange(square+8);
}

template <>
constexpr Square InitPlus8<Player::kBlack>(int square) {
    return InRange(square-8);
}

/**
 * @}
 */

/**
 * Get the square arrived at by making a pawn capture to the left
 *
 * @{
 */

template <Player P>
constexpr Square InitPlus9(int square) {
    return Square::Underflow;
}

template <>
constexpr Square InitPlus9<Player::kWhite>(int square) {
    return util::GetFile(square) < 7 ? InRange(square+9) : Square::Underflow;
}

template <>
constexpr Square InitPlus9<Player::kBlack>(int square) {
    return util::GetFile(square) > 0 ? InRange(square-9) : Square::Underflow;
}

/**
 * @}
 */

/**
 * Given an occupancy bitboard, compute a bishop's mobility
 *
 * @param [in] square   The square the bishop is on
 * @param [in] occupied The squares occupied by all other pieces
 *
 * @return The number of squares attacked by the bishop
 */
constexpr int MobilityDiag(int square, const std::uint64_t occupied) {
    return util::BitCount(AttacksFromDiag(square, occupied));
}

/**
 * Given an occupancy bitboard, compute a rook's mobility
 *
 * @param [in] square   The square the rook is on
 * @param [in] occupied The squares occupied by all other pieces
 *
 * @return The number of squares attacked by the rook
 */
constexpr int MobilityRook(int square, const std::uint64_t occupied) {
    return util::BitCount(AttacksFromRook(square, occupied));
}

/**
 * Get the number of bits set in the bishop "attacks from" bitboard at the
 * given table index
 *
 * @param[in] index The index computed by the magic bitboard hashing scheme
 *
 * @return The number of squares attacked by a bishop
 */
constexpr int InitMobilityDiag(std::uint32_t index) {

    // 1. Determine the square of the attacking bishop

    int from = 63;
    for (int i = 1; i < 64; i++) {
        if (DiagOffset(i) > index) {
            from = i-1; break;
        }
    }

    // 2. Find an occupancy board for which the hashing algorithm would yield
    //    the index passed to us

    const OccupancySet<512> set = GenDiagOccupancies(from);

    for (std::size_t i = 0; i < set.size; i++) {
        const std::uint32_t ind = DiagOffset(from) +
            ((DiagMagic(from) * set.table[i]) >> BishopDbShift(from));

        if (ind == index) return MobilityDiag(
            from, set.table[i]);
    }

    // This index is not used

    return 0;
}

/**
 * Get the number of bits set in the rook "attacks from" bitboard at the given
 * table index
 *
 * @param[in] index The index computed by the magic bitboard hashing scheme
 *
 * @return The number of squares attacked by a rook
 */
constexpr int InitMobilityRook(std::uint32_t index) {

    // 1. Determine the square of the attacking rook

    int from = 63;
    for (int i = 1; i < 64; i++) {
        if (RookOffset(i) > index) {
            from = i-1; break;
        }
    }

    // 2. Find an occupancy board for which the hashing algorithm would yield
    //    the index passed to us

    const OccupancySet<4096> set = GenRookOccupancies(from);

    for (std::size_t i = 0; i < set.size; i++) {
        const std::uint32_t ind = RookOffset(from) +
            ((RookMagic(from) * set.table[i]) >> RookDbShift(from));

        if (ind == index) return MobilityRook(
            from, set.table[i]);
    }

    // This index is not used

    return 0;
}

/**
 * Get the squares adjacent to and on the same rank as the given square
 *
 * @param[in] square The square whose adjacent squares to return
 *
 * @return A bitmask representing the squares adjacent to \a square
 */
constexpr std::uint64_t InitRankAdjacent(int square) {
    std::uint64_t ans = 0;

    if (util::GetFile(square) != 0)
        ans |= std::uint64_t(1) << (square-1);

    if (util::GetFile(square) != 7)
        ans |= std::uint64_t(1) << (square+1);

    return ans;
}

/**
 * Get a set of bits representing a ray with the specified origin that extends
 * through the given square to the edge of the board
 *
 * @param[in] origin The ray's origin square
 * @param[in] square The square through which to extend the ray
 *
 * @return A bitmask representing the desired ray
 */
constexpr std::uint64_t InitRay(int origin, int square) {
    std::uint64_t ans = std::uint64_t(1) << origin;

    switch (GetDirection(origin, square)) {
        case Direction::kAlongH1A8:
            if (origin < square)
                ans |= NorthWestMask(origin);
            else
                ans |= SouthEastMask(origin);
            break;
        case Direction::kAlongFile:
            if (origin < square)
                ans |= NorthMask(square);
            else
                ans |= SouthMask(square);
            break;
        case Direction::kAlongA1H8:
            if (origin < square)
                ans |= NorthEastMask(square);
            else
                ans |= SouthWestMask(square);
            break;
        case Direction::kAlongRank:
            if (origin < square)
                ans |= WestMask(square);
            else
                ans |= EastMask(square);
            break;
        default:
            // These squares do not connect
            ans = 0;
    }

    return ans;
}

/**
 * Get a bitboard that represents the entire file, rank or diagonal which
 * passes through the given two squares
 *
 * @param[in] square1 The first square
 * @param[in] square2 The second square
 *
 * @return The entire file, rank, or diagonal that includes \a square1 and \a
 *         square2, or zero if the squares do not connect
 */
constexpr std::uint64_t InitRayExtend(int square1, int square2) {
    if (square1 == square2)
        return 0;

    if (util::GetRank(square1) == util::GetRank(square2))
        return util::GetRankMask(square1);

    if (GetDiagH1A8(square1) == GetDiagH1A8(square2))
        return GetDiagH1A8(square1);

    if (util::GetFile(square1) == util::GetFile(square2))
        return util::GetFileMask(square1);

    if (GetDiagA1H8(square1) == GetDiagA1H8(square2))
        return GetDiagA1H8(square1);

    return 0;
}

/**
 * Get a bitboard representing all squares located between the given two
 * squares, excluding those two squares
 *
 * @param[in] square1 The first square
 * @param[in] square2 The second square
 *
 * @return A bitboard representing all squares in between \a square1 and \a
 *         square2, excluding these two. If these squares do not connect, then
 *         zero is returned
 */
constexpr std::uint64_t InitRaySegment(int square1, int square2) {
    return (NorthEastMask(square1) & SouthWestMask(square2)) |
           (NorthEastMask(square2) & SouthWestMask(square1)) |
           (NorthMask(square1) & SouthMask(square2)) |
           (NorthMask(square2) & SouthMask(square1)) |
           (NorthWestMask(square1) & SouthEastMask(square2)) |
           (NorthWestMask(square2) & SouthEastMask(square1)) |
           (EastMask(square1) & WestMask(square2))   |
           (WestMask(square1) & EastMask(square2));

}

}  // namespace internal
}  // namespace data_tables
}  // namespace chess

#endif  // DATA_TABLES_INTERNAL_H_
