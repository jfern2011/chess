/**
 *  \file   static_exchange_ut.cc
 *  \author Jason Fernandez
 *  \date   01/14/2023
 */

#include <cstddef>
#include <cstdint>
#include <vector>

#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "chess/chess.h"
#include "chess/position.h"
#include "chess/static_exchange.h"

// tests:
// 1) all piece types attacking/defending

namespace {
/**
 * @brief Maintains a set of pieces for a particular player
 */
class PieceSet {
public:
    PieceSet() : offset_(0), pieces_() {
    }

    PieceSet(const PieceSet& set) = default;
    PieceSet(PieceSet&& set) = default;
    PieceSet& operator=(const PieceSet& set) = default;
    PieceSet& operator=(PieceSet&& set) = default;

    ~PieceSet() = default;

    /**
     * @brief Get the next piece in the set
     *
     * @return The next piece, or EMPTY if no pieces are left
     */
    chess::Piece Next() {
        if (offset_ >= pieces_.size()) {
            return chess::Piece::EMPTY;
        } else {
            return pieces_.at(offset_++);
        }
    }

    /**
     * @brief Add a piece to the set
     *
     * @param piece The piece to add
     */
    void PushBack(chess::Piece piece) {
        pieces_.push_back(piece);
    }

    /**
     * @brief Get the number of pieces in this set
     *
     * @return The size of this set
     */
    std::size_t size() const {
        return pieces_.size();
    }

private:
    /**
     * Offset to the next piece to return
     */
    std::size_t offset_;

    /**
     * The pieces themselves
     */
    std::vector<chess::Piece> pieces_;
};

TEST(static_exchange, next_piece_diag) {
    chess::Position pos;

    ASSERT_EQ(pos.Reset("5k2/3b4/6p1/5p2/4P3/3B4/2Q5/5K2 w - - 0 1"),
              chess::Position::FenError::kSuccess);

    auto target = chess::Square::F5;
    std::uint64_t occupied = pos.Occupied();

    std::uint64_t attackers =
        pos.GetPlayerInfo<chess::Player::kWhite>().AttacksTo(target, occupied);
    std::uint64_t defenders =
        pos.GetPlayerInfo<chess::Player::kBlack>().AttacksTo(target, occupied);

    PieceSet white_pieces;
    PieceSet black_pieces;

    white_pieces.PushBack(chess::Piece::PAWN);
    white_pieces.PushBack(chess::Piece::BISHOP);
    white_pieces.PushBack(chess::Piece::QUEEN);

    black_pieces.PushBack(chess::Piece::PAWN);
    black_pieces.PushBack(chess::Piece::BISHOP);

    auto run_capture_sequence = [&]() -> void {
        auto turn = chess::Player::kWhite;

        const std::size_t max_iterations = white_pieces.size() +
                                           black_pieces.size();

        for (std::size_t i = 1; i <= max_iterations; i++) {
            if (turn == chess::Player::kWhite) {
                const chess::Piece next =
                    chess::detail::NextPiece<chess::Player::kWhite>(
                                    pos, target, &attackers, &defenders);

                ASSERT_EQ(white_pieces.Next(), next);
                turn = chess::Player::kBlack;
            } else {
                const chess::Piece next =
                    chess::detail::NextPiece<chess::Player::kBlack>(
                                    pos, target, &defenders, &attackers);

                ASSERT_EQ(black_pieces.Next(), next);
                turn = chess::Player::kWhite;
            }
        }
    };

    run_capture_sequence();

    // Make sure both piece sets were used up

    ASSERT_EQ(white_pieces.Next(), chess::Piece::EMPTY);
    ASSERT_EQ(black_pieces.Next(), chess::Piece::EMPTY);

    ASSERT_EQ(pos.Reset("5k2/8/6p1/5p2/4P3/3B4/2b5/1Q3K2 w - - 0 1"),
              chess::Position::FenError::kSuccess);

    target = chess::Square::F5;
    occupied = pos.Occupied();

    attackers =
        pos.GetPlayerInfo<chess::Player::kWhite>().AttacksTo(target, occupied);
    defenders =
        pos.GetPlayerInfo<chess::Player::kBlack>().AttacksTo(target, occupied);

    white_pieces = PieceSet();
    black_pieces = PieceSet();

    white_pieces.PushBack(chess::Piece::PAWN);
    white_pieces.PushBack(chess::Piece::BISHOP);
    white_pieces.PushBack(chess::Piece::QUEEN);

    black_pieces.PushBack(chess::Piece::PAWN);
    black_pieces.PushBack(chess::Piece::BISHOP);

    run_capture_sequence();

    // Make sure all piece sets were used up

    ASSERT_EQ(white_pieces.Next(), chess::Piece::EMPTY);
    ASSERT_EQ(black_pieces.Next(), chess::Piece::EMPTY);

    ASSERT_EQ(pos.Reset("5k2/7q/6p1/5p2/4P3/3B4/2b5/1Q3K2 w - - 0 1"),
              chess::Position::FenError::kSuccess);

    target = chess::Square::F5;
    occupied = pos.Occupied();

    attackers =
        pos.GetPlayerInfo<chess::Player::kWhite>().AttacksTo(target, occupied);
    defenders =
        pos.GetPlayerInfo<chess::Player::kBlack>().AttacksTo(target, occupied);

    white_pieces = PieceSet();
    black_pieces = PieceSet();

    white_pieces.PushBack(chess::Piece::PAWN);
    white_pieces.PushBack(chess::Piece::BISHOP);
    white_pieces.PushBack(chess::Piece::QUEEN);

    black_pieces.PushBack(chess::Piece::PAWN);
    black_pieces.PushBack(chess::Piece::BISHOP);
    black_pieces.PushBack(chess::Piece::QUEEN);

    run_capture_sequence();

    // Make sure all piece sets were used up

    ASSERT_EQ(white_pieces.Next(), chess::Piece::EMPTY);
    ASSERT_EQ(black_pieces.Next(), chess::Piece::EMPTY);

    ASSERT_EQ(pos.Reset("5k2/7q/6B1/5p2/4P3/8/2b5/1Q3K2 w - - 0 1"),
              chess::Position::FenError::kSuccess);

    target = chess::Square::F5;
    occupied = pos.Occupied();

    attackers =
        pos.GetPlayerInfo<chess::Player::kWhite>().AttacksTo(target, occupied);
    defenders =
        pos.GetPlayerInfo<chess::Player::kBlack>().AttacksTo(target, occupied);

    white_pieces = PieceSet();
    black_pieces = PieceSet();

    white_pieces.PushBack(chess::Piece::PAWN);
    white_pieces.PushBack(chess::Piece::BISHOP);
    white_pieces.PushBack(chess::Piece::QUEEN);

    black_pieces.PushBack(chess::Piece::BISHOP);
    black_pieces.PushBack(chess::Piece::QUEEN);

    run_capture_sequence();

    // Make sure all piece sets were used up

    ASSERT_EQ(white_pieces.Next(), chess::Piece::EMPTY);
    ASSERT_EQ(black_pieces.Next(), chess::Piece::EMPTY);
}

TEST(static_exchange, next_piece_rook) {
    chess::Position pos;

    ASSERT_EQ(pos.Reset("4k3/4r3/4r3/4q3/4p3/3PR1N1/4Q3/4K3 w - - 0 1"),
              chess::Position::FenError::kSuccess);

    auto target = chess::Square::E4;
    std::uint64_t occupied = pos.Occupied();

    std::uint64_t attackers =
        pos.GetPlayerInfo<chess::Player::kWhite>().AttacksTo(target, occupied);
    std::uint64_t defenders =
        pos.GetPlayerInfo<chess::Player::kBlack>().AttacksTo(target, occupied);

    PieceSet white_pieces;
    PieceSet black_pieces;

    white_pieces.PushBack(chess::Piece::PAWN);
    white_pieces.PushBack(chess::Piece::KNIGHT);
    white_pieces.PushBack(chess::Piece::ROOK);
    white_pieces.PushBack(chess::Piece::QUEEN);

    black_pieces.PushBack(chess::Piece::QUEEN);
    black_pieces.PushBack(chess::Piece::ROOK);
    black_pieces.PushBack(chess::Piece::ROOK);

    auto run_capture_sequence = [&]() -> void {
        auto turn = chess::Player::kWhite;

        const std::size_t max_iterations = white_pieces.size() +
                                           black_pieces.size();

        for (std::size_t i = 1; i <= max_iterations; i++) {
            if (turn == chess::Player::kWhite) {
                const chess::Piece next =
                    chess::detail::NextPiece<chess::Player::kWhite>(
                                    pos, target, &attackers, &defenders);

                ASSERT_EQ(white_pieces.Next(), next);
                turn = chess::Player::kBlack;
            } else {
                const chess::Piece next =
                    chess::detail::NextPiece<chess::Player::kBlack>(
                                    pos, target, &defenders, &attackers);

                ASSERT_EQ(black_pieces.Next(), next);
                turn = chess::Player::kWhite;
            }
        }
    };

    run_capture_sequence();

    // Make sure both piece sets were used up

    ASSERT_EQ(white_pieces.Next(), chess::Piece::EMPTY);
    ASSERT_EQ(black_pieces.Next(), chess::Piece::EMPTY);

    ASSERT_EQ(pos.Reset("7k/8/8/8/8/1qR1prQ1/4K3/8 w - - 0 1"),
              chess::Position::FenError::kSuccess);

    target = chess::Square::E3;
    occupied = pos.Occupied();

    attackers =
        pos.GetPlayerInfo<chess::Player::kWhite>().AttacksTo(target, occupied);
    defenders =
        pos.GetPlayerInfo<chess::Player::kBlack>().AttacksTo(target, occupied);

    white_pieces = PieceSet();
    black_pieces = PieceSet();

    white_pieces.PushBack(chess::Piece::ROOK);
    white_pieces.PushBack(chess::Piece::QUEEN);
    white_pieces.PushBack(chess::Piece::KING);

    black_pieces.PushBack(chess::Piece::ROOK);
    black_pieces.PushBack(chess::Piece::QUEEN);

    run_capture_sequence();

    // Make sure all piece sets were used up

    ASSERT_EQ(white_pieces.Next(), chess::Piece::EMPTY);
    ASSERT_EQ(black_pieces.Next(), chess::Piece::EMPTY);
}

TEST(static_exchange, next_piece_king) {
    chess::Position pos;

    ASSERT_EQ(pos.Reset("8/8/8/3p1pN1/7R/3k2N1/8/3K4 w - - 0 1"),
              chess::Position::FenError::kSuccess);

    auto target = chess::Square::E4;
    std::uint64_t occupied = pos.Occupied();

    std::uint64_t attackers =
        pos.GetPlayerInfo<chess::Player::kWhite>().AttacksTo(target, occupied);
    std::uint64_t defenders =
        pos.GetPlayerInfo<chess::Player::kBlack>().AttacksTo(target, occupied);

    PieceSet white_pieces;
    PieceSet black_pieces;

    white_pieces.PushBack(chess::Piece::KNIGHT);
    white_pieces.PushBack(chess::Piece::KNIGHT);
    white_pieces.PushBack(chess::Piece::ROOK);

    black_pieces.PushBack(chess::Piece::PAWN);
    black_pieces.PushBack(chess::Piece::PAWN);
    black_pieces.PushBack(chess::Piece::KING);

    auto run_capture_sequence = [&]() -> void {
        auto turn = chess::Player::kWhite;

        const std::size_t max_iterations = white_pieces.size() +
                                           black_pieces.size();

        for (std::size_t i = 1; i <= max_iterations; i++) {
            if (turn == chess::Player::kWhite) {
                const chess::Piece next =
                    chess::detail::NextPiece<chess::Player::kWhite>(
                                    pos, target, &attackers, &defenders);

                ASSERT_EQ(white_pieces.Next(), next);
                turn = chess::Player::kBlack;
            } else {
                const chess::Piece next =
                    chess::detail::NextPiece<chess::Player::kBlack>(
                                    pos, target, &defenders, &attackers);

                ASSERT_EQ(black_pieces.Next(), next);
                turn = chess::Player::kWhite;
            }
        }
    };

    run_capture_sequence();

    // Make sure both piece sets were used up

    ASSERT_EQ(white_pieces.Next(), chess::Piece::EMPTY);
    ASSERT_EQ(black_pieces.Next(), chess::Piece::EMPTY);
}

}  // namespace
