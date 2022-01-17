/**
 *  \file   util.cc
 *  \author Jason Fernandez
 *  \date   07/05/2020
 */

#include "chess/util.h"

#include <cctype>

#include "chess/data_tables.h"

namespace chess {
namespace util {

/**
 * Converts the character representation of a piece to its enumeration
 *
 * @note This is case-insensitive
 *
 * @param[in] piece The piece to convert
 *
 * @return The enum value corresponding to \a piece. If the conversion
 *         cannot be performed, \ref EMPTY is returned
 */
Piece CharToPiece(char piece) {
    const char lower = std::tolower(piece);
    switch (lower) {
        case 'p':
            return Piece::PAWN;
        case 'r':
            return Piece::ROOK;
        case 'n':
            return Piece::KNIGHT;
        case 'b':
            return Piece::BISHOP;
        case 'q':
            return Piece::QUEEN;
        case 'k':
            return Piece::KING;
        default:
            return Piece::EMPTY;
    }
}

/**
 * @brief Get the zero-indexed least significant bit (LSB) set
 *
 * @param qword The word whose LSB to compute
 *
 * @return The index of the least significant bit, or -1 if no bits are set
 */
constexpr std::int8_t Lsb(std::uint64_t qword) noexcept {
    qword &= (-qword);

    if (qword < 0x0000000010000ull) return 00 + data_tables::kLsb[qword >> 00];
    if (qword < 0x0000100000000ull) return 16 + data_tables::kLsb[qword >> 16];
    if (qword < 0x1000000000000ull) return 32 + data_tables::kLsb[qword >> 32];
	    
    return 48 + data_tables::kLsb[qword >> 48];
}

/**
 * @brief Get the zero-indexed most significant bit (MSB) set
 *
 * @param qword The word whose MSB to compute
 *
 * @return The index of the most significant bit, or -1 if no bits are set
 */
constexpr std::int8_t Msb(std::uint64_t qword) noexcept {
    if (qword < 0x0000000010000ull) return 00 + data_tables::kMsb[qword >> 00];
    if (qword < 0x0000100000000ull) return 16 + data_tables::kMsb[qword >> 16];
    if (qword < 0x1000000000000ull) return 32 + data_tables::kMsb[qword >> 32];

    return 48 + data_tables::kMsb[qword >> 48];
}

/**
 * Convert a \ref Piece enumeration to a human-readable representation
 *
 * @param[in] piece    The piece to convert
 * @param[in] to_lower If true, convert to lower case
 *
 * @return The character equivalent of \a piece
 */
char PieceToChar(Piece piece, bool to_lower) {
    char c;
    switch (piece) {
        case Piece::PAWN:
            c = 'P'; break;
        case Piece::ROOK:
            c = 'R'; break;
        case Piece::KNIGHT:
            c = 'N'; break;
        case Piece::BISHOP:
            c = 'B'; break;
        case Piece::QUEEN:
            c = 'Q'; break;
        case Piece::KING:
            c = 'K'; break;
        case Piece::EMPTY:
            c = ' '; break;
        default:
            c = '?';
    }

    return to_lower ? std::tolower(c) : c;
}

/**
 * Convert the string representation of a square to its enumeration
 *
 * @param[in] str The string to convert
 *
 * @return The square enum value, or \ref Overflow on error
 */
Square StrToSquare(const std::string& str) {
    for (auto square = Square::H1; square <= Square::A8; square++) {
        if (str == kSquareStr[square]) {
            return square;
        }
    }

    // The square could not be mapped
    return Square::Overflow;
}

}  // namespace util
}  // namespace chess
