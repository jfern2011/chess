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
constexpr std::int16_t kKingValue   = 32000;

constexpr std::int16_t kKnightValue = 325;
constexpr std::int16_t kPawnValue   = 100;
constexpr std::int16_t kBishopValue = 325;
constexpr std::int16_t kQueenValue  = 975;
constexpr std::int16_t kRookValue   = 500;
constexpr std::int16_t kEmptyValue  = 0;

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

extern const char* kSquareStr[65];

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

struct PieceEnum {
    enum Type {
        pawn,
        rook,
        knight,
        bishop,
        queen,
        king,
        empty
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

}  // namespace chess

#endif  // CHESS_H_
