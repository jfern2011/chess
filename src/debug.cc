/**
 *  \file   debug.cc
 *  \author Jason Fernandez
 *  \date   04/04/2020
 */

#include "chess/debug.h"
#include "chess/util.h"

namespace chess {
namespace debug {

/**
 * Return the given 64-bit integer as an 8x8 bit array
 *
 * @param[in] board The bitboard
 */
std::string PrintBitBoard(std::uint64_t board)
{
    std::string out;

    int prev_rank = 8;
    for (int square = 63; square >= -1; square--) {
        if (util::GetRank(square) != prev_rank) {
            out += "\n ---+---+---+---+---+---+---+--- \n";
            if (square == -1) break;

            prev_rank = util::GetRank(square);
        }

        if (board & (std::uint64_t(1) << square)) {
            out += "| * ";
        } else {
            out += "|   ";
        }

        if (square % 8 == 0) out += "|";
    }

    return (out + "\n");
}

/**
 * Convert a 21-bit move to its human-readable form
 *
 * @param[in] move The move to interpret
 *
 * @return The parsed move info
 */
std::string PrintMove(std::int32_t move) {
    Piece captured = util::ExtractCaptured(move);
    Square orig = util::ExtractFrom(move);
    Piece moved = util::ExtractMoved(move);
    Piece promoted = util::ExtractPromoted(move);
    Square dest = util::ExtractTo(move);

    promoted = promoted == Piece::PAWN ? Piece::EMPTY : promoted;

    auto pieceToStr = [](Piece piece) -> std::string {
        if (piece == Piece::KING) return "king";
        if (piece == Piece::PAWN) return "pawn";
        if (piece == Piece::KNIGHT) return "knight";
        if (piece == Piece::BISHOP) return "bishop";
        if (piece == Piece::ROOK) return "rook";
        if (piece == Piece::QUEEN) return "queen";
        if (piece == Piece::EMPTY) return "";
        return "??";
    };

    std::string output("Move:");
    output += "\n\tcaptured = " + pieceToStr(captured);
    output += "\n\tfrom     = " + std::string(kSquareStr[orig]);
    output += "\n\tmoved    = " + pieceToStr(moved);
    output += "\n\tpromoted = " + pieceToStr(promoted);
    output += "\n\tto       = " + std::string(kSquareStr[dest]);

    return output;
}

}  // namespace debug
}  // namespace chess
