/**
 *  \file   data_tables.h
 *  \author Jason Fernandez
 *  \date   11/02/2019
 */

#ifndef CHESS_DATA_TABLES_H_
#define CHESS_DATA_TABLES_H_

#include <array>
#include <cstdint>

#include "chess/chess.h"
#include "chess/private/data_tables_internal.h"

namespace chess {
namespace data_tables {

/**
 * The "3rd" rank, as seen from each player's perspective
 *
 * @{
 */

template <Player P>
auto k3rdRank = std::uint64_t(0);

template<> constexpr auto k3rdRank<Player::kBlack> = kRank6;

template<> constexpr auto k3rdRank<Player::kWhite> = kRank3;

/**
 * @}
 */

/**
 * Bitboards representing an a1-h8 diagonal that a given square lies on
 */
constexpr auto kA1H8_64 = internal::CreateTable<64>(internal::GetDiagA1H8);

/**
 * Bitmasks representing the back rank for each side
 *
 * @{
 */

template <Player P>
auto kBackRank = std::uint64_t(0);

template<> constexpr auto kBackRank<Player::kBlack> = kRank8;

template<> constexpr auto kBackRank<Player::kWhite> = kRank1;

/**
 * @}
 */

/**
 * A database containing the "attacks from" bitboards for a bishop
 */
constexpr auto kBishopAttacks = internal::InitAttacksFromDiag();

/**
 * The occupancy squares we mask the occupied squares bitboard with to obtain
 * a key into the \ref bishop_attacks database
 */
constexpr auto kBishopAttacksMask =
    internal::CreateTable<64>(internal::BishopOccupancyMask);

/**
 * The amount we need to bit shift to obtain an index into the \ref
 * bishop_attacks database
 */
constexpr auto kBishopDbShifts =
    internal::CreateTable<64>(internal::BishopDbShift);

/**
 * A database that contains the mobility of bishops, as a function of square
 * and occupancy. A higher mobility score indicates the bishop can move to more
 * squares
 */
constexpr auto kBishopMobility = internal::InitMobilityDiag();

/**
 * Offset into the \ref bishop_attacks database that marks the start of the
 * "attacks from" boards for a particular square
 */
constexpr auto kBishopOffsets =
    internal::CreateTable<64>(internal::DiagOffset);

/**
 * All squares reachable by a bishop from a given square including the square
 * itself
 */
constexpr auto kBishopRangeMask =
    internal::CreateTable<64>(internal::BishopRangeMask);

/**
 * The destination square when castling long
 *
 * @{
 */

template <Player P>
auto kCastleLongDest = Square::Underflow;

template<> constexpr auto kCastleLongDest<Player::kWhite> = Square::C1;

template<> constexpr auto kCastleLongDest<Player::kBlack> = Square::C8;

/**
 * @}
 */

/**
 * The squares a king must traverse to castle long
 *
 * @{
 */

template <Player P>
auto kCastleLongPath =
    std::array<Square,2>{ Square::Underflow, Square::Underflow };

template<> constexpr auto kCastleLongPath<Player::kWhite> =
    std::array<Square,2>{ Square::D1, Square::C1 };

template<> constexpr auto kCastleLongPath<Player::kBlack> =
    std::array<Square,2>{ Square::D8, Square::C8 };

/**
 * @}
 */

/**
 * The destination square when castling short
 *
 * @{
 */

template <Player P>
auto kCastleShortDest = Square::Underflow;

template<> constexpr auto kCastleShortDest<Player::kWhite> = Square::G1;

template<> constexpr auto kCastleShortDest<Player::kBlack> = Square::G8;

/**
 * @}
 */

/**
 * The squares a king must traverse to castle short
 *
 * @{
 */

template <Player P>
auto kCastleShortPath =
    std::array<Square,2>{ Square::Underflow, Square::Underflow };

template<> constexpr auto kCastleShortPath<Player::kWhite> =
    std::array<Square,2>{ Square::F1, Square::G1 };

template<> constexpr auto kCastleShortPath<Player::kBlack> =
    std::array<Square,2>{ Square::F8, Square::G8 };

/**
 * @}
 */

/**
 * Bitmasks used to clear single bits. All bits are set except at the
 * corresponding index
 */
constexpr auto kClearMask = internal::CreateTable<64>(internal::ClearMask);

/**
 * The magic numbers used to look up "attacks from" boards for bishops in the
 * magic bitboard hashing algorithm
 */
constexpr auto kDiagMagics = internal::CreateTable<64>(internal::DiagMagic);

/**
 * Describes how two squares are connected (along a file, along a diagonal,
 * etc.)
 *
 * @note They are NOT connected if they are the same square
 */
constexpr auto kDirections =
    internal::CreateTable<64,64>(internal::GetDirection);

/**
 * All squares "east" of a particular square, from white's perspective
 */
constexpr auto kEastMask =  internal::CreateTable<64>(internal::EastMask);

/**
 * En-passant target squares. These are invalid except for the 4th and 5th
 * 5th ranks; e.g. kEpTarget[E4] = E3, kEpTarget[E5] = E6
 */
constexpr auto kEpTarget = internal::CreateTable<64>(internal::EpTarget);

/**
 * Material exchange[piece captured][piece moved]. A positive value indicates
 * a definite material gain
 *
 * @note The 7th "piece" is an empty square
 */
constexpr std::array<std::array<std::int16_t,7>,7> kExchange = {{{
        kPawnValue - kPawnValue,
        kPawnValue - kRookValue,
        kPawnValue - kKnightValue,
        kPawnValue - kBishopValue,
        kPawnValue - kQueenValue,
        kPawnValue - kKingValue,
        kPawnValue - kEmptyValue
    }, {
        kRookValue - kPawnValue,
        kRookValue - kRookValue,
        kRookValue - kKnightValue,
        kRookValue - kBishopValue,
        kRookValue - kQueenValue,
        kRookValue - kKingValue,
        kRookValue - kEmptyValue
    }, {
        kKnightValue - kPawnValue,
        kKnightValue - kRookValue,
        kKnightValue - kKnightValue,
        kKnightValue - kBishopValue,
        kKnightValue - kQueenValue,
        kKnightValue - kKingValue,
        kKnightValue - kEmptyValue
    }, {
        kBishopValue - kPawnValue,
        kBishopValue - kRookValue,
        kBishopValue - kKnightValue,
        kBishopValue - kBishopValue,
        kBishopValue - kQueenValue,
        kBishopValue - kKingValue,
        kBishopValue - kEmptyValue
    }, {
        kQueenValue - kPawnValue,
        kQueenValue - kRookValue,
        kQueenValue - kKnightValue,
        kQueenValue - kBishopValue,
        kQueenValue - kQueenValue,
        kQueenValue - kKingValue,
        kQueenValue - kEmptyValue
    }, {
        kKingValue - kPawnValue,
        kKingValue - kRookValue,
        kKingValue - kKnightValue,
        kKingValue - kBishopValue,
        kKingValue - kQueenValue,
        kKingValue - kKingValue,
        kKingValue - kEmptyValue
    }, {
        kEmptyValue - kPawnValue,
        kEmptyValue - kRookValue,
        kEmptyValue - kKnightValue,
        kEmptyValue - kBishopValue,
        kEmptyValue - kQueenValue,
        kEmptyValue - kKingValue,
        kEmptyValue - kEmptyValue
    }
}};

/**
 * Bitboards representing the file that a given square lies on
 */
constexpr auto kFiles64 = internal::CreateTable<64>(util::GetFileMask);

/**
 * Bitboards representing an a1-h8 diagonal that a given square lies on
 */
constexpr auto kH1A8_64 = internal::CreateTable<64>(internal::GetDiagH1A8);

/**
 * Database of "attacks from" bitboards for a king
 */
constexpr auto kKingAttacks =
    internal::CreateTable<64>(internal::InitAttacksFromKing);

/**
 * The home squares of both kings (e1, e8)
 *
 * @{
 */

template <Player P>
auto kKingHome = Square::Underflow;

template<> constexpr auto kKingHome<Player::kWhite> = Square::E1;

template<> constexpr auto kKingHome<Player::kBlack> = Square::E8;

/**
 * @}
 */

/**
 * Bits representing the kingside, i.e. for white this would be F1 and G1
 *
 * @{
 */

template <Player P>
std::uint64_t kKingSide = 0;

template<> constexpr auto kKingSide<Player::kWhite> =
    jfern::bitops::create_mask<std::uint64_t, Square::F1, Square::G1>();

template<> constexpr auto kKingSide<Player::kBlack> =
    jfern::bitops::create_mask<std::uint64_t, Square::F8, Square::G8>();

/**
 * @}
 */

/**
 * Database of "attacks from" bitboards for a knight
 */
constexpr auto kKnightAttacks =
    internal::CreateTable<64>(internal::InitAttacksFromKnight);

/**
 * Returns the LSB for every possible unsigned 16-bit value
 */
constexpr auto kLsb =
    internal::CreateTable<65536>(jfern::bitops::lsb<std::uint16_t>);

/**
 * The square arrived at by retreating 2 pawn steps
 *
 * @{
 */

template <Player P>
auto kMinus16 = std::array<Square,64>();

template<> constexpr auto kMinus16<Player::kWhite> =
    internal::CreateTable<64>(internal::InitMinus16<Player::kWhite>);

template<> constexpr auto kMinus16<Player::kBlack> =
    internal::CreateTable<64>(internal::InitMinus16<Player::kBlack>);

/**
 * @}
 */

/**
 * The square arrived at via undoing a pawn capture to the right
 *
 * @{
 */

template <Player P>
auto kMinus7 = std::array<Square,64>();

template<> constexpr auto kMinus7<Player::kWhite> =
    internal::CreateTable<64>(internal::InitMinus7<Player::kWhite>);

template<> constexpr auto kMinus7<Player::kBlack> =
    internal::CreateTable<64>(internal::InitMinus7<Player::kBlack>);

/**
 * @}
 */

/**
 * The square arrived at by retreating 1 pawn step
 *
 * @{
 */

template <Player P>
auto kMinus8 = std::array<Square,64>();

template<> constexpr auto kMinus8<Player::kWhite> =
    internal::CreateTable<64>(internal::InitMinus8<Player::kWhite>);

template<> constexpr auto kMinus8<Player::kBlack> =
    internal::CreateTable<64>(internal::InitMinus8<Player::kBlack>);

/**
 * @}
 */

/**
 * The square arrived at via undoing a pawn capture to the left
 *
 * @{
 */

template <Player P>
auto kMinus9 = std::array<Square,64>();

template<> constexpr auto kMinus9<Player::kWhite> =
    internal::CreateTable<64>(internal::InitMinus9<Player::kWhite>);

template<> constexpr auto kMinus9<Player::kBlack> =
    internal::CreateTable<64>(internal::InitMinus9<Player::kBlack>);

/**
 * @}
 */

/**
 * Returns the MSB for every possible unsigned 16-bit value
 */
constexpr auto kMsb =
    internal::CreateTable<65536>(jfern::bitops::msb<std::uint16_t>);

/**
 * All squares "north" of a particular square, from white's perspective
 */
constexpr auto kNorthMask = internal::CreateTable<64>(internal::NorthMask);

/**
 * All squares "northeast" of a particular square, from white's perspective
 */
constexpr auto kNorthEastMask =
    internal::CreateTable<64>(internal::NorthEastMask);

/**
 * All squares "northwest" of a particular square, from white's perspective
 */
constexpr auto kNorthWestMask =
    internal::CreateTable<64>(internal::NorthWestMask);

/**
 * Database of the squares that a pawn can advance to
 *
 * @{
 */

template <Player P>
constexpr auto kPawnAdvances = std::array<std::uint64_t, 64>();

template<>
constexpr auto kPawnAdvances<Player::kWhite> =
    internal::CreateTable<64>(internal::InitPawnAdvances<Player::kWhite>);

template<>
constexpr auto kPawnAdvances<Player::kBlack> =
    internal::CreateTable<64>(internal::InitPawnAdvances<Player::kBlack>);

/**
 * @}
 */

/**
 * Database of squares attacked by a pawn from a particular square
 *
 * @{
 */

template <Player P>
constexpr auto kPawnAttacks = std::array<std::uint64_t, 64>();

template<>
constexpr auto kPawnAttacks<Player::kWhite> =
    internal::CreateTable<64>(internal::InitPawnAttacks<Player::kWhite>);

template<>
constexpr auto kPawnAttacks<Player::kBlack> =
    internal::CreateTable<64>(internal::InitPawnAttacks<Player::kBlack>);

/**
 * @}
 */

/**
 * The value of each type of piece
 */
constexpr std::array<std::int16_t,7> kPieceValue = {{
        kPawnValue,
        kRookValue,
        kKnightValue,
        kBishopValue,
        kQueenValue,
        kKingValue,
        kEmptyValue
    }
};

/**
 * The square arrived at by advancing 2 pawn steps
 *
 * @{
 */

template <Player P>
auto kPlus16 = std::array<Square,64>();

template<> constexpr auto kPlus16<Player::kWhite> =
    internal::CreateTable<64>(internal::InitPlus16<Player::kWhite>);

template<> constexpr auto kPlus16<Player::kBlack> =
    internal::CreateTable<64>(internal::InitPlus16<Player::kBlack>);

/**
 * @}
 */

/**
 * The square arrived at via making a pawn capture to the right
 *
 * @{
 */

template <Player P>
auto kPlus7 = std::array<Square,64>();

template<> constexpr auto kPlus7<Player::kWhite> =
    internal::CreateTable<64>(internal::InitPlus7<Player::kWhite>);

template<> constexpr auto kPlus7<Player::kBlack> =
    internal::CreateTable<64>(internal::InitPlus7<Player::kBlack>);

/**
 * @}
 */

/**
 * The square arrived at by advancing 1 pawn step
 *
 * @{
 */

template <Player P>
auto kPlus8 = std::array<Square,64>();

template<> constexpr auto kPlus8<Player::kWhite> =
    internal::CreateTable<64>(internal::InitPlus8<Player::kWhite>);

template<> constexpr auto kPlus8<Player::kBlack> =
    internal::CreateTable<64>(internal::InitPlus8<Player::kBlack>);

/**
 * @}
 */

/**
 * The square arrived at via making a pawn capture to the left
 *
 * @{
 */

template <Player P>
auto kPlus9 = std::array<Square,64>();

template<> constexpr auto kPlus9<Player::kWhite> =
    internal::CreateTable<64>(internal::InitPlus9<Player::kWhite>);

template<> constexpr auto kPlus9<Player::kBlack> =
    internal::CreateTable<64>(internal::InitPlus9<Player::kBlack>);

/**
 * @}
 */

/**
 * Returns the population count for every possible unsigned 16-bit value
 */
constexpr auto kPop =
    internal::CreateTable<65536>(jfern::bitops::count<std::uint16_t>);

/**
 * Bitmasks representing the queenside
 *
 * @{
 */

template <Player P>
auto kQueenside = std::uint64_t(0);

template<> constexpr auto kQueenside<Player::kWhite> =
    jfern::bitops::create_mask<std::uint64_t, Square::B1,
                                              Square::C1,
                                              Square::D1>();

template<> constexpr auto kQueenside<Player::kBlack> =
    jfern::bitops::create_mask<std::uint64_t, Square::B8,
                                              Square::C8,
                                              Square::D8>();

/**
 * @}
 */

/**
 * Bitmasks indicating the squares adjacent to each square and on the same rank
 *
 * E.g. rank_adjacent[E4] = D4 | F4
 */
constexpr auto kRankAdjacent =
    internal::CreateTable<64>(internal::InitRankAdjacent);

/**
 * Bitboard representing the rank that a given square lies on:
 */
constexpr auto kRanks64 = internal::CreateTable<64>(util::GetRankMask);

/**
 * Describes a ray whose origin is at the 1st index and extends to the end of
 * the board through the 2nd
 */
constexpr auto kRay = internal::CreateTable<64,64>(internal::InitRay);

/**
 * Similar to kRaySegment, but includes the entire "line" along that direction,
 * e.g.
 *
 * kRayExtend[B2][C3] = entire A1-H8 diagonal
 */
constexpr auto kRayExtend =
    internal::CreateTable<64,64>(internal::InitRayExtend);

/**
 * Represents all squares located between any two squares, but excluding those
 * two squares
 */
constexpr auto kRaySegment =
    internal::CreateTable<64,64>(internal::InitRaySegment);

/**
 * A database containing the "attacks from" bitboards for a rook
 */
constexpr auto kRookAttacks = internal::InitAttacksFromRook();

/**
 * The occupancy squares we mask the occupied squares bitboard with to obtain
 * a key into the \ref rook_attacks database
 */
constexpr auto kRookAttacksMask =
    internal::CreateTable<64>(internal::RookOccupancyMask);

/**
 * The amount we need to bit shift to obtain an index into the \ref
 * rook_attacks database
 */
constexpr auto kRookDbShifts =
    internal::CreateTable<64>(internal::RookDbShift);

/**
 * The magic numbers used to look up "attacks_from" boards for rooks in the
 * magic bitboard hashing algorithm
 */
constexpr auto kRookMagics = internal::CreateTable<64>(internal::RookMagic);

/**
 * A database that contains the mobility of rooks, as a function of square
 * and occupancy. A higher mobility score indicates the rook can move to more
 * squares
 */
constexpr auto kRookMobility = internal::InitMobilityRook();

/**
 * Offset into the \ref rook_attacks database that marks the start of the
 * "attacks from" boards for a particular square
 */
constexpr auto kRookOffsets =
    internal::CreateTable<64>(internal::RookOffset);

/**
 * All squares reachable by a rook from a given square including the square
 * itself
 */
constexpr auto kRookRangeMask =
    internal::CreateTable<64>(internal::RookRangeMask);

/**
 * All squares "south" of a particular square, from white's perspective
 */
constexpr auto kSouthMask = internal::CreateTable<64>(internal::SouthMask);

/**
 * All squares "southeast" of a particular square, from white's perspective
 */
constexpr auto kSouthEastMask =
    internal::CreateTable<64>(internal::SouthEastMask);

/**
 * All squares "southwest" of a particular square, from white's perspective
 */
constexpr auto kSouthWestMask =
    internal::CreateTable<64>(internal::SouthWestMask);

/**
 * All squares "west" of a particular square, from white's perspective
 */
constexpr auto kWestMask = internal::CreateTable<64>(internal::WestMask);

}  // namespace data_tables
}  // namespace chess

#endif  // CHESS_DATA_TABLES_H_
