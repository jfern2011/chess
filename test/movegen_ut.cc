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

TEST(MoveGen, GeneratePawnCaptures) {
    auto pos = chess::Position();
    EXPECT_EQ(pos.Reset("2r5/1P5k/8/8/2p1r3/3P4/7K/8 w - - 0 1"),
              chess::Position::FenError::kSuccess);

    std::uint64_t pinned = pos.PinnedPieces<chess::Player::kWhite>();
    std::uint64_t target =
        ~pos.GetPlayerInfo<chess::Player::kWhite>().Occupied();

    std::array<std::uint32_t, 256> moves{0};

    std::size_t n_moves = chess::GeneratePawnCaptures<chess::Player::kWhite>(
                            pos, target, pinned, moves.data());

    EXPECT_EQ(n_moves, 10u);

    std::vector<std::uint32_t> expected = {
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
                              chess::Square::B8),
        chess::util::PackMove(chess::Piece::ROOK,
                              chess::Square::B7,
                              chess::Piece::PAWN,
                              chess::Piece::ROOK,
                              chess::Square::C8),
        chess::util::PackMove(chess::Piece::ROOK,
                              chess::Square::B7,
                              chess::Piece::PAWN,
                              chess::Piece::KNIGHT,
                              chess::Square::C8),
        chess::util::PackMove(chess::Piece::ROOK,
                              chess::Square::B7,
                              chess::Piece::PAWN,
                              chess::Piece::BISHOP,
                              chess::Square::C8),
        chess::util::PackMove(chess::Piece::ROOK,
                              chess::Square::B7,
                              chess::Piece::PAWN,
                              chess::Piece::QUEEN,
                              chess::Square::C8),
        chess::util::PackMove(chess::Piece::PAWN,
                              chess::Square::D3,
                              chess::Piece::PAWN,
                              chess::Piece::EMPTY,
                              chess::Square::C4),
        chess::util::PackMove(chess::Piece::ROOK,
                              chess::Square::D3,
                              chess::Piece::PAWN,
                              chess::Piece::EMPTY,
                              chess::Square::E4)
    };

    for (std::uint32_t move : expected) {
        auto iter = std::find(expected.begin(), expected.end(), move);
        EXPECT_NE(iter, expected.end());
    }

    EXPECT_EQ(pos.Reset("2r5/KP4qk/8/8/2p1r3/3P4/8/8 w - - 0 1"),
              chess::Position::FenError::kSuccess);

    pinned = pos.PinnedPieces<chess::Player::kWhite>();
    target = ~pos.GetPlayerInfo<chess::Player::kWhite>().Occupied();

    moves.fill(0);

    n_moves = chess::GeneratePawnCaptures<chess::Player::kWhite>(
                pos, target, pinned, moves.data());

    EXPECT_EQ(n_moves, 2u);

    expected = {
        chess::util::PackMove(chess::Piece::PAWN,
                              chess::Square::D3,
                              chess::Piece::PAWN,
                              chess::Piece::EMPTY,
                              chess::Square::C4),
        chess::util::PackMove(chess::Piece::ROOK,
                              chess::Square::D3,
                              chess::Piece::PAWN,
                              chess::Piece::EMPTY,
                              chess::Square::E4)
    };

    for (std::uint32_t move : expected) {
        auto iter = std::find(expected.begin(), expected.end(), move);
        EXPECT_NE(iter, expected.end());
    }

    EXPECT_EQ(pos.Reset("8/7k/8/8/2p5/2KP3r/8/8 w - - 0 1"),
              chess::Position::FenError::kSuccess);

    pinned = pos.PinnedPieces<chess::Player::kWhite>();
    target = ~pos.GetPlayerInfo<chess::Player::kWhite>().Occupied();

    moves.fill(0);

    n_moves = chess::GeneratePawnCaptures<chess::Player::kWhite>(
                pos, target, pinned, moves.data());

    EXPECT_EQ(n_moves, 0u);

    EXPECT_EQ(pos.Reset("8/7k/8/8/2p1b3/3P4/2K5/8 w - - 0 1"),
              chess::Position::FenError::kSuccess);

    pinned = pos.PinnedPieces<chess::Player::kWhite>();
    target = ~pos.GetPlayerInfo<chess::Player::kWhite>().Occupied();

    moves.fill(0);

    n_moves = chess::GeneratePawnCaptures<chess::Player::kWhite>(
                pos, target, pinned, moves.data());

    EXPECT_EQ(n_moves, 1u);

    expected = {
        chess::util::PackMove(chess::Piece::BISHOP,
                              chess::Square::D3,
                              chess::Piece::PAWN,
                              chess::Piece::EMPTY,
                              chess::Square::E4)
    };

    for (std::uint32_t move : expected) {
        auto iter = std::find(expected.begin(), expected.end(), move);
        EXPECT_NE(iter, expected.end());
    }

    EXPECT_EQ(pos.Reset("8/7k/8/1q6/4b3/3P4/4K3/8 w - - 0 1"),
              chess::Position::FenError::kSuccess);

    pinned = pos.PinnedPieces<chess::Player::kWhite>();
    target = ~pos.GetPlayerInfo<chess::Player::kWhite>().Occupied();

    moves.fill(0);

    n_moves = chess::GeneratePawnCaptures<chess::Player::kWhite>(
                pos, target, pinned, moves.data());

    EXPECT_EQ(n_moves, 0u);
}

TEST(MoveGen, EnPassantCaptures) {
    auto pos = chess::Position();
    EXPECT_EQ(pos.Reset("4k1b1/8/8/3PpP2/2K5/8/8/8 w - e6 0 1"),
              chess::Position::FenError::kSuccess);

    std::uint64_t pinned = pos.PinnedPieces<chess::Player::kWhite>();
    std::uint64_t target =
        ~pos.GetPlayerInfo<chess::Player::kWhite>().Occupied();

    std::array<std::uint32_t, 256> moves{0};

    std::size_t n_moves = chess::GeneratePawnCaptures<chess::Player::kWhite>(
                            pos, target, pinned, moves.data());

    EXPECT_EQ(n_moves, 2u);

    std::vector<std::uint32_t> expected = {
        chess::util::PackMove(chess::Piece::PAWN,
                              chess::Square::D5,
                              chess::Piece::PAWN,
                              chess::Piece::EMPTY,
                              chess::Square::E6),
        chess::util::PackMove(chess::Piece::PAWN,
                              chess::Square::F5,
                              chess::Piece::PAWN,
                              chess::Piece::EMPTY,
                              chess::Square::E6)
    };

    for (std::uint32_t move : expected) {
        auto iter = std::find(expected.begin(), expected.end(), move);
        EXPECT_NE(iter, expected.end());
    }

    const std::vector<std::string> fens = {
        "3rk3/8/8/3PpP2/8/8/8/3K4 w - e6 0 1",
        "b3k3/8/8/3PpP2/4K3/8/8/8 w - e6 0 1"
    };

    for (const std::string& fen : fens) {
        EXPECT_EQ(pos.Reset(fen), chess::Position::FenError::kSuccess);

        pinned = pos.PinnedPieces<chess::Player::kWhite>();
        target = ~pos.GetPlayerInfo<chess::Player::kWhite>().Occupied();

        moves.fill(0);

        n_moves = chess::GeneratePawnCaptures<chess::Player::kWhite>(
                    pos, target, pinned, moves.data());

        EXPECT_EQ(n_moves, 1u);

        expected = {
            chess::util::PackMove(chess::Piece::PAWN,
                                  chess::Square::F5,
                                  chess::Piece::PAWN,
                                  chess::Piece::EMPTY,
                                  chess::Square::E6)
        };

        for (std::uint32_t move : expected) {
            auto iter = std::find(expected.begin(), expected.end(), move);
            EXPECT_NE(iter, expected.end());
        }
    }

    EXPECT_EQ(pos.Reset("4k3/8/8/2KPpr2/8/8/8/8 w - e6 0 1"),
              chess::Position::FenError::kSuccess);

    pinned = pos.PinnedPieces<chess::Player::kWhite>();
    target = ~pos.GetPlayerInfo<chess::Player::kWhite>().Occupied();

    moves.fill(0);

    n_moves = chess::GeneratePawnCaptures<chess::Player::kWhite>(
                pos, target, pinned, moves.data());

    EXPECT_EQ(n_moves, 0u);
}

TEST(MoveGen, GenerateCaptures) {
    auto pos = chess::Position();
    EXPECT_EQ(pos.Reset("4k3/8/8/2Q3B1/8/2R1r3/3P1KN1/8 w - - 0 1"),
              chess::Position::FenError::kSuccess);

    std::uint64_t pinned = pos.PinnedPieces<chess::Player::kWhite>();

    std::array<std::uint32_t, 256> moves{0};

    std::size_t n_moves = chess::GenerateCaptures<chess::Player::kWhite>(
                            pos, pinned, moves.data());

    EXPECT_EQ(n_moves, 6u);

    std::vector<std::uint32_t> expected = {
        chess::util::PackMove(chess::Piece::ROOK,
                              chess::Square::F2,
                              chess::Piece::KING,
                              chess::Piece::EMPTY,
                              chess::Square::E3),
        chess::util::PackMove(chess::Piece::ROOK,
                              chess::Square::D2,
                              chess::Piece::PAWN,
                              chess::Piece::EMPTY,
                              chess::Square::E3),
        chess::util::PackMove(chess::Piece::ROOK,
                              chess::Square::C3,
                              chess::Piece::ROOK,
                              chess::Piece::EMPTY,
                              chess::Square::E3),
        chess::util::PackMove(chess::Piece::ROOK,
                              chess::Square::C5,
                              chess::Piece::QUEEN,
                              chess::Piece::EMPTY,
                              chess::Square::E3),
        chess::util::PackMove(chess::Piece::ROOK,
                              chess::Square::G5,
                              chess::Piece::BISHOP,
                              chess::Piece::EMPTY,
                              chess::Square::E3),
        chess::util::PackMove(chess::Piece::ROOK,
                              chess::Square::G2,
                              chess::Piece::KNIGHT,
                              chess::Piece::EMPTY,
                              chess::Square::E3)
    };

    for (std::uint32_t move : expected) {
        auto iter = std::find(expected.begin(), expected.end(), move);
        EXPECT_NE(iter, expected.end());
    }
}

TEST(MoveGen, GenerateNonCaptures) {
    auto pos = chess::Position();
    EXPECT_EQ(pos.Reset("4k3/5p2/8/7B/2p2p2/p1PP1P2/P1Q1N2P/R3K2R w KQ - 0 1"),
              chess::Position::FenError::kSuccess);

    std::uint64_t pinned = pos.PinnedPieces<chess::Player::kWhite>();

    std::array<std::uint32_t, 256> moves{0};

    std::size_t n_moves = chess::GenerateNonCaptures<chess::Player::kWhite>(
                            pos, pinned, moves.data());

    std::vector<std::uint32_t> expected = {
        chess::util::PackMove(chess::Piece::EMPTY,
                              chess::Square::A1,
                              chess::Piece::ROOK,
                              chess::Piece::EMPTY,
                              chess::Square::B1),
        chess::util::PackMove(chess::Piece::EMPTY,
                              chess::Square::A1,
                              chess::Piece::ROOK,
                              chess::Piece::EMPTY,
                              chess::Square::C1),
        chess::util::PackMove(chess::Piece::EMPTY,
                              chess::Square::A1,
                              chess::Piece::ROOK,
                              chess::Piece::EMPTY,
                              chess::Square::D1),
        chess::util::PackMove(chess::Piece::EMPTY,
                              chess::Square::H1,
                              chess::Piece::ROOK,
                              chess::Piece::EMPTY,
                              chess::Square::G1),
        chess::util::PackMove(chess::Piece::EMPTY,
                              chess::Square::H1,
                              chess::Piece::ROOK,
                              chess::Piece::EMPTY,
                              chess::Square::F1),
        chess::util::PackMove(chess::Piece::EMPTY,
                              chess::Square::C2,
                              chess::Piece::QUEEN,
                              chess::Piece::EMPTY,
                              chess::Square::A4),
        chess::util::PackMove(chess::Piece::EMPTY,
                              chess::Square::C2,
                              chess::Piece::QUEEN,
                              chess::Piece::EMPTY,
                              chess::Square::B3),
        chess::util::PackMove(chess::Piece::EMPTY,
                              chess::Square::C2,
                              chess::Piece::QUEEN,
                              chess::Piece::EMPTY,
                              chess::Square::B2),
        chess::util::PackMove(chess::Piece::EMPTY,
                              chess::Square::C2,
                              chess::Piece::QUEEN,
                              chess::Piece::EMPTY,
                              chess::Square::D2),
        chess::util::PackMove(chess::Piece::EMPTY,
                              chess::Square::C2,
                              chess::Piece::QUEEN,
                              chess::Piece::EMPTY,
                              chess::Square::B1),
        chess::util::PackMove(chess::Piece::EMPTY,
                              chess::Square::C2,
                              chess::Piece::QUEEN,
                              chess::Piece::EMPTY,
                              chess::Square::C1),
        chess::util::PackMove(chess::Piece::EMPTY,
                              chess::Square::C2,
                              chess::Piece::QUEEN,
                              chess::Piece::EMPTY,
                              chess::Square::D1),
        chess::util::PackMove(chess::Piece::EMPTY,
                              chess::Square::E2,
                              chess::Piece::KNIGHT,
                              chess::Piece::EMPTY,
                              chess::Square::D4),
        chess::util::PackMove(chess::Piece::EMPTY,
                              chess::Square::E2,
                              chess::Piece::KNIGHT,
                              chess::Piece::EMPTY,
                              chess::Square::G3),
        chess::util::PackMove(chess::Piece::EMPTY,
                              chess::Square::E2,
                              chess::Piece::KNIGHT,
                              chess::Piece::EMPTY,
                              chess::Square::G1),
        chess::util::PackMove(chess::Piece::EMPTY,
                              chess::Square::E2,
                              chess::Piece::KNIGHT,
                              chess::Piece::EMPTY,
                              chess::Square::C1),
        chess::util::PackMove(chess::Piece::EMPTY,
                              chess::Square::H5,
                              chess::Piece::BISHOP,
                              chess::Piece::EMPTY,
                              chess::Square::G4),
        chess::util::PackMove(chess::Piece::EMPTY,
                              chess::Square::H5,
                              chess::Piece::BISHOP,
                              chess::Piece::EMPTY,
                              chess::Square::G6),
        chess::util::PackMove(chess::Piece::EMPTY,
                              chess::Square::D3,
                              chess::Piece::PAWN,
                              chess::Piece::EMPTY,
                              chess::Square::D4),
        chess::util::PackMove(chess::Piece::EMPTY,
                              chess::Square::H2,
                              chess::Piece::PAWN,
                              chess::Piece::EMPTY,
                              chess::Square::H3),
        chess::util::PackMove(chess::Piece::EMPTY,
                              chess::Square::H2,
                              chess::Piece::PAWN,
                              chess::Piece::EMPTY,
                              chess::Square::H4),
        chess::util::PackMove(chess::Piece::EMPTY,
                              chess::Square::E1,
                              chess::Piece::KING,
                              chess::Piece::EMPTY,
                              chess::Square::D1),
        chess::util::PackMove(chess::Piece::EMPTY,
                              chess::Square::E1,
                              chess::Piece::KING,
                              chess::Piece::EMPTY,
                              chess::Square::D2),
        chess::util::PackMove(chess::Piece::EMPTY,
                              chess::Square::E1,
                              chess::Piece::KING,
                              chess::Piece::EMPTY,
                              chess::Square::F1),
        chess::util::PackMove(chess::Piece::EMPTY,
                              chess::Square::E1,
                              chess::Piece::KING,
                              chess::Piece::EMPTY,
                              chess::Square::F2),
        chess::util::PackMove(chess::Piece::EMPTY,
                              chess::Square::E1,
                              chess::Piece::KING,
                              chess::Piece::EMPTY,
                              chess::Square::G1),
        chess::util::PackMove(chess::Piece::EMPTY,
                              chess::Square::E1,
                              chess::Piece::KING,
                              chess::Piece::EMPTY,
                              chess::Square::C1)
    };

    EXPECT_EQ(n_moves, expected.size())
        << PrintMoves(moves.data(), n_moves);

    for (std::uint32_t move : expected) {
        auto iter = std::find(expected.begin(), expected.end(), move);
        EXPECT_NE(iter, expected.end());
    }
}

TEST(MoveGen, GenerateCastleMoves) {
    auto pos = chess::Position();
    EXPECT_EQ(pos.Reset("3rk3/8/8/8/8/8/6p1/R3K2R w KQ - 0 1"),
              chess::Position::FenError::kSuccess);

    std::array<std::uint32_t, 256> moves{0};

    std::size_t n_moves = chess::GenerateCastleMoves<chess::Player::kWhite>(
                            pos, moves.data());

    EXPECT_EQ(n_moves, 0u);

    EXPECT_EQ(pos.Reset("4k3/8/8/8/8/4n3/8/R3K2R w KQ - 0 1"),
              chess::Position::FenError::kSuccess);

    moves.fill(0);

    n_moves = chess::GenerateCastleMoves<chess::Player::kWhite>(
                pos, moves.data());

    EXPECT_EQ(n_moves, 0u);

    EXPECT_EQ(pos.Reset("4k3/8/8/8/8/q6n/8/R3K2R w KQ - 0 1"),
              chess::Position::FenError::kSuccess);

    moves.fill(0);

    n_moves = chess::GenerateCastleMoves<chess::Player::kWhite>(
                pos, moves.data());

    EXPECT_EQ(n_moves, 0u);

    EXPECT_EQ(pos.Reset("8/8/8/8/2b5/8/2k5/R3K2R w KQ - 0 1"),
              chess::Position::FenError::kSuccess);

    moves.fill(0);

    n_moves = chess::GenerateCastleMoves<chess::Player::kWhite>(
                pos, moves.data());

    EXPECT_EQ(n_moves, 0u);
}

TEST(MoveGen, GenerateKingMoves) {
    auto pos = chess::Position();
    EXPECT_EQ(pos.Reset("4k3/1n6/p3b3/8/2K4r/8/8/4q3 w - - 0 1"),
              chess::Position::FenError::kSuccess);

    std::array<std::uint32_t, 256> moves{0};

    std::size_t n_moves = chess::GenerateKingMoves<chess::Player::kWhite>(
                            pos, ~pos.Occupied(), moves.data());

    std::vector<std::uint32_t> expected = {
        chess::util::PackMove(chess::Piece::EMPTY,
                              chess::Square::C4,
                              chess::Piece::KING,
                              chess::Piece::EMPTY,
                              chess::Square::D3)
    };

    EXPECT_EQ(n_moves, expected.size());

    for (std::uint32_t move : expected) {
        auto iter = std::find(expected.begin(), expected.end(), move);
        EXPECT_NE(iter, expected.end());
    }

    EXPECT_EQ(pos.Reset("3r4/8/p7/8/2K5/8/2k5/6b1 w - - 0 1"),
              chess::Position::FenError::kSuccess);

    moves.fill(0);

    n_moves = chess::GenerateKingMoves<chess::Player::kWhite>(
                pos, ~pos.Occupied(), moves.data());

    expected = {
        chess::util::PackMove(chess::Piece::EMPTY,
                              chess::Square::C4,
                              chess::Piece::KING,
                              chess::Piece::EMPTY,
                              chess::Square::B4)
    };

    EXPECT_EQ(n_moves, expected.size());

    for (std::uint32_t move : expected) {
        auto iter = std::find(expected.begin(), expected.end(), move);
        EXPECT_NE(iter, expected.end());
    }
}

TEST(MoveGen, GenerateCheckEvasions) {
    auto pos = chess::Position();
    EXPECT_EQ(pos.Reset("4k3/1n6/p3b3/8/2K4r/8/8/4q3 w - - 0 1"),
              chess::Position::FenError::kSuccess);

    std::array<std::uint32_t, 256> moves{0};

    std::size_t n_moves = chess::GenerateCheckEvasions<chess::Player::kWhite>(
                            pos, &moves);

    std::vector<std::uint32_t> expected = {
        chess::util::PackMove(chess::Piece::EMPTY,
                              chess::Square::C4,
                              chess::Piece::KING,
                              chess::Piece::EMPTY,
                              chess::Square::D3)
    };

    EXPECT_EQ(n_moves, expected.size());

    for (std::uint32_t move : expected) {
        auto iter = std::find(expected.begin(), expected.end(), move);
        EXPECT_NE(iter, expected.end());
    }

    EXPECT_EQ(pos.Reset("4k1B1/1b6/7R/8/3PK3/2N5/Q7/8 w - - 0 1"),
              chess::Position::FenError::kSuccess);

    moves.fill(0);

    n_moves = chess::GenerateCheckEvasions<chess::Player::kWhite>(
                pos, &moves);

    expected = {
        chess::util::PackMove(chess::Piece::EMPTY,
                              chess::Square::E4,
                              chess::Piece::KING,
                              chess::Piece::EMPTY,
                              chess::Square::D3),
        chess::util::PackMove(chess::Piece::EMPTY,
                              chess::Square::E4,
                              chess::Piece::KING,
                              chess::Piece::EMPTY,
                              chess::Square::E3),
        chess::util::PackMove(chess::Piece::EMPTY,
                              chess::Square::E4,
                              chess::Piece::KING,
                              chess::Piece::EMPTY,
                              chess::Square::F4),
        chess::util::PackMove(chess::Piece::EMPTY,
                              chess::Square::E4,
                              chess::Piece::KING,
                              chess::Piece::EMPTY,
                              chess::Square::F5),
        chess::util::PackMove(chess::Piece::EMPTY,
                              chess::Square::E4,
                              chess::Piece::KING,
                              chess::Piece::EMPTY,
                              chess::Square::E5),
        chess::util::PackMove(chess::Piece::EMPTY,
                              chess::Square::D4,
                              chess::Piece::PAWN,
                              chess::Piece::EMPTY,
                              chess::Square::D5),
        chess::util::PackMove(chess::Piece::EMPTY,
                              chess::Square::C3,
                              chess::Piece::KNIGHT,
                              chess::Piece::EMPTY,
                              chess::Square::D5),
        chess::util::PackMove(chess::Piece::EMPTY,
                              chess::Square::A2,
                              chess::Piece::QUEEN,
                              chess::Piece::EMPTY,
                              chess::Square::D5),
        chess::util::PackMove(chess::Piece::EMPTY,
                              chess::Square::G8,
                              chess::Piece::BISHOP,
                              chess::Piece::EMPTY,
                              chess::Square::D5),
        chess::util::PackMove(chess::Piece::EMPTY,
                              chess::Square::H6,
                              chess::Piece::PAWN,
                              chess::Piece::EMPTY,
                              chess::Square::C6)
    };

    EXPECT_EQ(n_moves, expected.size());

    for (std::uint32_t move : expected) {
        auto iter = std::find(expected.begin(), expected.end(), move);
        EXPECT_NE(iter, expected.end());
    }

    EXPECT_EQ(pos.Reset("4k3/2b5/8/8/8/8/r5PK/8 w - - 0 1"),
              chess::Position::FenError::kSuccess);

    moves.fill(0);

    n_moves = chess::GenerateCheckEvasions<chess::Player::kWhite>(
                pos, &moves);

    expected = {
        chess::util::PackMove(chess::Piece::EMPTY,
                              chess::Square::H2,
                              chess::Piece::KING,
                              chess::Piece::EMPTY,
                              chess::Square::H1),
        chess::util::PackMove(chess::Piece::EMPTY,
                              chess::Square::H2,
                              chess::Piece::KING,
                              chess::Piece::EMPTY,
                              chess::Square::H3),
        chess::util::PackMove(chess::Piece::EMPTY,
                              chess::Square::H2,
                              chess::Piece::KING,
                              chess::Piece::EMPTY,
                              chess::Square::G1)
    };

    EXPECT_EQ(n_moves, expected.size());

    for (std::uint32_t move : expected) {
        auto iter = std::find(expected.begin(), expected.end(), move);
        EXPECT_NE(iter, expected.end());
    }

    EXPECT_EQ(pos.Reset("4k3/8/8/8/8/7r/q5PK/8 w - - 0 1"),
              chess::Position::FenError::kSuccess);

    moves.fill(0);

    n_moves = chess::GenerateCheckEvasions<chess::Player::kWhite>(
                pos, &moves);

    expected = {
        chess::util::PackMove(chess::Piece::ROOK,
                              chess::Square::H2,
                              chess::Piece::KING,
                              chess::Piece::EMPTY,
                              chess::Square::H3),
        chess::util::PackMove(chess::Piece::EMPTY,
                              chess::Square::H2,
                              chess::Piece::KING,
                              chess::Piece::EMPTY,
                              chess::Square::G1)
    };

    EXPECT_EQ(n_moves, expected.size());

    for (std::uint32_t move : expected) {
        auto iter = std::find(expected.begin(), expected.end(), move);
        EXPECT_NE(iter, expected.end());
    }

    EXPECT_EQ(pos.Reset("B3k3/Q7/8/8/1PK4q/1PP1P3/3P1R2/8 w - - 0 1"),
              chess::Position::FenError::kSuccess);

    moves.fill(0);

    n_moves = chess::GenerateCheckEvasions<chess::Player::kWhite>(
                pos, &moves);

    expected = {
        chess::util::PackMove(chess::Piece::EMPTY,
                              chess::Square::F2,
                              chess::Piece::ROOK,
                              chess::Piece::EMPTY,
                              chess::Square::F4),
        chess::util::PackMove(chess::Piece::EMPTY,
                              chess::Square::E3,
                              chess::Piece::PAWN,
                              chess::Piece::EMPTY,
                              chess::Square::E4),
        chess::util::PackMove(chess::Piece::EMPTY,
                              chess::Square::D2,
                              chess::Piece::PAWN,
                              chess::Piece::EMPTY,
                              chess::Square::D4),
        chess::util::PackMove(chess::Piece::EMPTY,
                              chess::Square::A7,
                              chess::Piece::QUEEN,
                              chess::Piece::EMPTY,
                              chess::Square::D4),
        chess::util::PackMove(chess::Piece::EMPTY,
                              chess::Square::A8,
                              chess::Piece::BISHOP,
                              chess::Piece::EMPTY,
                              chess::Square::E4),
        chess::util::PackMove(chess::Piece::EMPTY,
                              chess::Square::C4,
                              chess::Piece::KING,
                              chess::Piece::EMPTY,
                              chess::Square::B5),
        chess::util::PackMove(chess::Piece::EMPTY,
                              chess::Square::C4,
                              chess::Piece::KING,
                              chess::Piece::EMPTY,
                              chess::Square::C5),
        chess::util::PackMove(chess::Piece::EMPTY,
                              chess::Square::C4,
                              chess::Piece::KING,
                              chess::Piece::EMPTY,
                              chess::Square::D5)
    };

    EXPECT_EQ(n_moves, expected.size());

    for (std::uint32_t move : expected) {
        auto iter = std::find(expected.begin(), expected.end(), move);
        EXPECT_NE(iter, expected.end());
    }

    EXPECT_EQ(pos.Reset("3k4/4r3/B5Q1/8/8/3n3R/2PPPP2/2N1K2R w K - 0 1"),
              chess::Position::FenError::kSuccess);

    moves.fill(0);

    n_moves = chess::GenerateCheckEvasions<chess::Player::kWhite>(
                pos, &moves);

    expected = {
        chess::util::PackMove(chess::Piece::KNIGHT,
                              chess::Square::C2,
                              chess::Piece::PAWN,
                              chess::Piece::EMPTY,
                              chess::Square::D3),
        chess::util::PackMove(chess::Piece::KNIGHT,
                              chess::Square::C1,
                              chess::Piece::KNIGHT,
                              chess::Piece::EMPTY,
                              chess::Square::D3),
        chess::util::PackMove(chess::Piece::KNIGHT,
                              chess::Square::H3,
                              chess::Piece::ROOK,
                              chess::Piece::EMPTY,
                              chess::Square::D3),
        chess::util::PackMove(chess::Piece::KNIGHT,
                              chess::Square::G7,
                              chess::Piece::QUEEN,
                              chess::Piece::EMPTY,
                              chess::Square::D3),
        chess::util::PackMove(chess::Piece::KNIGHT,
                              chess::Square::A6,
                              chess::Piece::BISHOP,
                              chess::Piece::EMPTY,
                              chess::Square::D3),
        chess::util::PackMove(chess::Piece::EMPTY,
                              chess::Square::E1,
                              chess::Piece::KING,
                              chess::Piece::EMPTY,
                              chess::Square::D1),
        chess::util::PackMove(chess::Piece::EMPTY,
                              chess::Square::E1,
                              chess::Piece::KING,
                              chess::Piece::EMPTY,
                              chess::Square::F1)
    };

    EXPECT_EQ(n_moves, expected.size());

    for (std::uint32_t move : expected) {
        auto iter = std::find(expected.begin(), expected.end(), move);
        EXPECT_NE(iter, expected.end());
    }
}

}  // namespace
