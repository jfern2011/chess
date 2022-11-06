/**
 *  \file   movegen_ut.cc
 *  \author Jason Fernandez
 *  \date   11/05/2022
 */

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

#include "gtest/gtest.h"

#include "chess/debug.h"
#include "chess/movegen.h"
#include "chess/position.h"

namespace {
/**
 * @brief Print each element in a list of moves
 *
 * @param moves The moves to print
 * @param size  The number of moves in the list
 *
 * @return The printed set of moves
 */
std::string PrintMoves(const std::uint32_t* moves, std::size_t size) {
    std::string out;

    for (std::size_t i = 0; i < size; i++) {
        out += chess::debug::PrintMove(moves[i]) + "\n";
    }

    return out;
}

TEST(MoveGen, GeneratePawnAdvances) {
    auto pos = chess::Position();
    EXPECT_EQ(pos.Reset("4r2b/4P3/5P2/1q1PKP1r/3P4/4P3/1q1Pr3/5k2 w - - 0 1"),
              chess::Position::FenError::kSuccess);

    std::uint64_t pinned = pos.PinnedPieces<chess::Player::kWhite>();
    std::uint64_t target = ~pos.Occupied();

    std::array<std::uint32_t, 256> moves{0};

    std::size_t n_moves = chess::GeneratePawnAdvances<chess::Player::kWhite>(
                            pos, target, pinned, moves.data());

    EXPECT_EQ(n_moves, 2u);

    std::vector<std::uint32_t> expected = {
        chess::util::PackMove(chess::Piece::EMPTY,
                              chess::Square::D2,
                              chess::Piece::PAWN,
                              chess::Piece::EMPTY,
                              chess::Square::D3),
        chess::util::PackMove(chess::Piece::EMPTY,
                              chess::Square::E3,
                              chess::Piece::PAWN,
                              chess::Piece::EMPTY,
                              chess::Square::E4)
    };

    for (std::uint32_t move : expected) {
        auto iter = std::find(expected.begin(), expected.end(), move);
        EXPECT_NE(iter, expected.end());
    }

    const std::vector<std::string> fens = {
        "8/1P2KP1r/8/8/6k1/8/4P3/8 w - - 0 1",
        "4K3/1P3P2/8/7b/6k1/8/4P3/8 w - - 0 1",
        "6b1/1P3P2/4K3/8/6k1/8/4P3/8 w - - 0 1"
    };

    for (const std::string& fen : fens) {
        EXPECT_EQ(pos.Reset(fen), chess::Position::FenError::kSuccess);

        std::uint64_t pinned = pos.PinnedPieces<chess::Player::kWhite>();
        std::uint64_t target = ~pos.Occupied();

        moves.fill(0);

        n_moves = chess::GeneratePawnAdvances<chess::Player::kWhite>(
                        pos, target, pinned, moves.data());

        ASSERT_EQ(n_moves, 6u) << PrintMoves(moves.data(), n_moves);

        std::vector<std::uint32_t> expected = {
            chess::util::PackMove(chess::Piece::EMPTY,
                                  chess::Square::E2,
                                  chess::Piece::PAWN,
                                  chess::Piece::EMPTY,
                                  chess::Square::E3),
            chess::util::PackMove(chess::Piece::EMPTY,
                                  chess::Square::E2,
                                  chess::Piece::PAWN,
                                  chess::Piece::EMPTY,
                                  chess::Square::E4),
            chess::util::PackMove(chess::Piece::EMPTY,
                                  chess::Square::B7,
                                  chess::Piece::PAWN,
                                  chess::Piece::ROOK,
                                  chess::Square::B8),
            chess::util::PackMove(chess::Piece::EMPTY,
                                  chess::Square::B7,
                                  chess::Piece::PAWN,
                                  chess::Piece::KNIGHT,
                                  chess::Square::B8),
            chess::util::PackMove(chess::Piece::EMPTY,
                                  chess::Square::B7,
                                  chess::Piece::PAWN,
                                  chess::Piece::BISHOP,
                                  chess::Square::B8),
            chess::util::PackMove(chess::Piece::EMPTY,
                                  chess::Square::B7,
                                  chess::Piece::PAWN,
                                  chess::Piece::QUEEN,
                                  chess::Square::B8)
        };

        for (std::uint32_t move : expected) {
            auto iter = std::find(expected.begin(), expected.end(), move);
            EXPECT_NE(iter, expected.end());
        }
    }
}
}  // namespace
