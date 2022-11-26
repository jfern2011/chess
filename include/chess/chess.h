/**
 *  \file   chess.h
 *  \author Jason Fernandez
 *  \date   11/02/2019
 */

#ifndef CHESS_H_
#define CHESS_H_

#include <cstdint>

namespace chess {

constexpr std::uint32_t kMaxMoves   = 256;
constexpr std::uint32_t kMaxPly     = 512;

/**
 * Assign the king a large value that still fits within
 * 16 bits (signed)
 */
constexpr std::int16_t kKingValue   = 12000;

constexpr std::int16_t kKnightValue = 325;
constexpr std::int16_t kPawnValue   = 100;
constexpr std::int16_t kBishopValue = 325;
constexpr std::int16_t kQueenValue  = 975;
constexpr std::int16_t kRookValue   = 500;
constexpr std::int16_t kEmptyValue  = 0;

constexpr std::uint32_t kNullMove = 0;

constexpr std::uint64_t kRank1 = 0x00000000000000ff;
constexpr std::uint64_t kRank2 = kRank1 <<  8;
constexpr std::uint64_t kRank3 = kRank1 << 16;
constexpr std::uint64_t kRank4 = kRank1 << 24;
constexpr std::uint64_t kRank5 = kRank1 << 32;
constexpr std::uint64_t kRank6 = kRank1 << 40;
constexpr std::uint64_t kRank7 = kRank1 << 48;
constexpr std::uint64_t kRank8 = kRank1 << 56;

constexpr std::uint64_t kFileH = 0x0101010101010101;
constexpr std::uint64_t kFileG = kFileH << 1;
constexpr std::uint64_t kFileF = kFileH << 2;
constexpr std::uint64_t kFileE = kFileH << 3;
constexpr std::uint64_t kFileD = kFileH << 4;
constexpr std::uint64_t kFileC = kFileH << 5;
constexpr std::uint64_t kFileB = kFileH << 6;
constexpr std::uint64_t kFileA = kFileH << 7;

constexpr const char* kSquareStr[65] = {
    "h1","g1","f1","e1","d1","c1","b1","a1",
    "h2","g2","f2","e2","d2","c2","b2","a2",
    "h3","g3","f3","e3","d3","c3","b3","a3",
    "h4","g4","f4","e4","d4","c4","b4","a4",
    "h5","g5","f5","e5","d5","c5","b5","a5",
    "h6","g6","f6","e6","d6","c6","b6","a6",
    "h7","g7","f7","e7","d7","c7","b7","a7",
    "h8","g8","f8","e8","d8","c8","b8","a8",
    "??"
};

enum class Direction {
    kAlongRank,
    kAlongFile,
    kAlongA1H8,
    kAlongH1A8,
    kNone
};

enum class Player {
    kBlack,
    kWhite,
    kBoth
};

/** @note Used for array indexing. Do NOT modify */
struct PieceEnum {
    enum Type {
        PAWN   = 0,
        ROOK   = 1,
        KNIGHT = 2,
        BISHOP = 3,
        QUEEN  = 4,
        KING   = 5,
        EMPTY  = 6
    };
};

using Piece = PieceEnum::Type;

enum class Increment {
    kMinus1,
    kMinus7,
    kMinus8,
    kMinus9,
    kPlus1,
    kPlus7,
    kPlus8,
    kPlus9
};

struct SquareEnum {
    enum Type {
        Underflow = -1,
        H1, G1, F1, E1, D1, C1, B1, A1,
        H2, G2, F2, E2, D2, C2, B2, A2,
        H3, G3, F3, E3, D3, C3, B3, A3,
        H4, G4, F4, E4, D4, C4, B4, A4,
        H5, G5, F5, E5, D5, C5, B5, A5,
        H6, G6, F6, E6, D6, C6, B6, A6,
        H7, G7, F7, E7, D7, C7, B7, A7,
        H8, G8, F8, E8, D8, C8, B8, A8,
        Overflow
    };
};

using Square = SquareEnum::Type;

/**
 * Postfix increment for \ref Square
 */
constexpr Square operator++(Square& square, int) noexcept {
    return square = static_cast<Square>(square+1);
}

/**
 * Postfix decrement for \ref Square
 */
constexpr Square operator--(Square& square, int) noexcept {
    return square = static_cast<Square>(square-1);
}

/**
 * Operator to increment a \ref Square
 *
 * @note Does NOT do bounds checking
 *
 * @param[in] square The square to increment
 * @param[in] i      The amount by which to increment \a square
 *
 * @return The incremented square
 */
constexpr Square operator+(Square square, int i) noexcept {
    return static_cast<Square>(int(square) + i);
}

/**
 * Operator to decrement a \ref Square
 *
 * @note Does NOT do bounds checking
 *
 * @param[in] square The square to decrement
 * @param[in] i      The amount by which to decrement \a square
 *
 * @return The decremented square
 */
constexpr Square operator-(Square square, int i) noexcept {
    return static_cast<Square>(int(square) - i);
}

}  // namespace chess

#endif  // CHESS_H_
