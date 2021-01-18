/**
 *  \file   position_ut.cc
 *  \author Jason Fernandez
 *  \date   07/03/2020
 */

#include <algorithm>
#include <map>
#include <ostream>
#include <sstream>
#include <string>
#include <tuple>
#include <vector>

#include "gtest/gtest.h"

#include "chess/debug.h"
#include "chess/position.h"

namespace {

/**
 * Helper functions useful for setting up a test position
 *
 * @{
 */
template <chess::Player player, chess::Piece piece>
void GivePieces(chess::Position::PlayerInfo<player>& ) {
}

template <chess::Player player, chess::Piece piece, chess::Square square,
          chess::Square... squares>
void GivePieces(chess::Position::PlayerInfo<player>& info) {
    info.template Drop<piece>(square);
    GivePieces<player, piece, squares...>(info);
}
/**
 * @}
 */

TEST(Position, PieceSet_Get) {
    auto set = chess::Position::PieceSet();
    EXPECT_THROW(set.Get<chess::Piece::EMPTY>(), std::logic_error);
    
    set.pieces64[chess::Piece::PAWN]   = std::uint64_t(0xfeed);
    set.pieces64[chess::Piece::ROOK]   = std::uint64_t(0xcafe);
    set.pieces64[chess::Piece::KNIGHT] = std::uint64_t(0xdeaf);
    set.pieces64[chess::Piece::BISHOP] = std::uint64_t(0xface);
    set.pieces64[chess::Piece::QUEEN]  = std::uint64_t(0xdead);
    set.pieces64[chess::Piece::KING]   = std::uint64_t(0xbeef);

    EXPECT_EQ(set.Get<chess::Piece::PAWN>(),   0xfeedu);
    EXPECT_EQ(set.Get<chess::Piece::ROOK>(),   0xcafeu);
    EXPECT_EQ(set.Get<chess::Piece::KNIGHT>(), 0xdeafu);
    EXPECT_EQ(set.Get<chess::Piece::BISHOP>(), 0xfaceu);
    EXPECT_EQ(set.Get<chess::Piece::QUEEN>(),  0xdeadu);
    EXPECT_EQ(set.Get<chess::Piece::KING>(),   0xbeefu);
}

TEST(Position, PieceSet_Put) {
    auto set = chess::Position::PieceSet();
    EXPECT_THROW(set.Put<chess::Piece::EMPTY>(chess::Square::E4),
                 std::logic_error);

    set.Put<chess::Piece::PAWN>(chess::Square::E4);
    set.Put<chess::Piece::ROOK>(chess::Square::F1);
    set.Put<chess::Piece::KNIGHT>(chess::Square::G8);
    set.Put<chess::Piece::BISHOP>(chess::Square::B6);
    set.Put<chess::Piece::QUEEN>(chess::Square::H5);
    set.Put<chess::Piece::KING>(chess::Square::A2);

    constexpr auto one = std::uint64_t(1);

    EXPECT_EQ(set.pieces64[chess::Piece::PAWN],   one << chess::Square::E4);
    EXPECT_EQ(set.pieces64[chess::Piece::ROOK],   one << chess::Square::F1);
    EXPECT_EQ(set.pieces64[chess::Piece::KNIGHT], one << chess::Square::G8);
    EXPECT_EQ(set.pieces64[chess::Piece::BISHOP], one << chess::Square::B6);
    EXPECT_EQ(set.pieces64[chess::Piece::QUEEN],  one << chess::Square::H5);
    EXPECT_EQ(set.pieces64[chess::Piece::KING],   one << chess::Square::A2);
    
    EXPECT_EQ(set.king_square[chess::Piece::KING], chess::Square::A2);
}

TEST(Position, PlayerInfo_AttacksTo) {
    // FEN: 3R4/1BN1N3/1N2QN2/R3K3/1NP1PN2/2N1N3/3Q4/7k w - - 0 1
    chess::Position::PlayerInfo<chess::Player::kWhite> info;

    GivePieces<chess::Player::kWhite,
               chess::Piece::PAWN,
               chess::Square::C4,
               chess::Square::E4>(info);

    GivePieces<chess::Player::kWhite,
               chess::Piece::ROOK,
               chess::Square::D8,
               chess::Square::A5>(info);

    GivePieces<chess::Player::kWhite,
               chess::Piece::KNIGHT,
               chess::Square::C7,
               chess::Square::E7,
               chess::Square::B6,
               chess::Square::F6,
               chess::Square::B4,
               chess::Square::F4,
               chess::Square::C3,
               chess::Square::E3>(info);

    GivePieces<chess::Player::kWhite,
               chess::Piece::BISHOP,
               chess::Square::B7>(info);

    GivePieces<chess::Player::kWhite,
               chess::Piece::QUEEN,
               chess::Square::E6,
               chess::Square::D2>(info);

    GivePieces<chess::Player::kWhite,
               chess::Piece::KING,
               chess::Square::E5>(info);

    constexpr auto expect =
        (std::uint64_t(1) << chess::Square::C4) |
        (std::uint64_t(1) << chess::Square::E4) |
        (std::uint64_t(1) << chess::Square::D8) |
        (std::uint64_t(1) << chess::Square::A5) |
        (std::uint64_t(1) << chess::Square::C7) |
        (std::uint64_t(1) << chess::Square::E7) |
        (std::uint64_t(1) << chess::Square::B6) |
        (std::uint64_t(1) << chess::Square::F6) |
        (std::uint64_t(1) << chess::Square::B4) |
        (std::uint64_t(1) << chess::Square::F4) |
        (std::uint64_t(1) << chess::Square::C3) |
        (std::uint64_t(1) << chess::Square::E3) |
        (std::uint64_t(1) << chess::Square::B7) |
        (std::uint64_t(1) << chess::Square::E6) |
        (std::uint64_t(1) << chess::Square::D2) |
        (std::uint64_t(1) << chess::Square::E5);

    const auto actual = info.AttacksTo(chess::Square::D5);

    EXPECT_EQ(expect, actual) << "Actual:\n"
                              << chess::debug::PrintBitBoard(actual)
                              << "Expected:\n"
                              << chess::debug::PrintBitBoard(expect)
                              << std::endl;
}

TEST(Position, PlayerInfo_Bishops) {
    chess::Position::PlayerInfo<chess::Player::kWhite> info;

    info.Drop<chess::Piece::BISHOP>(chess::Square::E5);

    EXPECT_EQ(info.Bishops(), chess::util::GetBit(chess::Square::E5));
}

TEST(Position, PlayerInfo_King) {
    chess::Position::PlayerInfo<chess::Player::kWhite> info;

    info.Drop<chess::Piece::KING>(chess::Square::E5);

    EXPECT_EQ(info.King(), chess::util::GetBit(chess::Square::E5));
    EXPECT_EQ(info.KingSquare(), chess::Square::E5);
}

TEST(Position, PlayerInfo_Knights) {
    chess::Position::PlayerInfo<chess::Player::kWhite> info;

    info.Drop<chess::Piece::KNIGHT>(chess::Square::E5);

    EXPECT_EQ(info.Knights(), chess::util::GetBit(chess::Square::E5));
}

TEST(Position, PlayerInfo_Pawns) {
    chess::Position::PlayerInfo<chess::Player::kWhite> info;

    info.Drop<chess::Piece::PAWN>(chess::Square::E5);

    EXPECT_EQ(info.Pawns(), chess::util::GetBit(chess::Square::E5));
}

TEST(Position, PlayerInfo_Rooks) {
    chess::Position::PlayerInfo<chess::Player::kWhite> info;

    info.Drop<chess::Piece::ROOK>(chess::Square::E5);

    EXPECT_EQ(info.Rooks(), chess::util::GetBit(chess::Square::E5));
}

TEST(Position, PlayerInfo_Queens) {
    chess::Position::PlayerInfo<chess::Player::kWhite> info;

    info.Drop<chess::Piece::QUEEN>(chess::Square::E5);

    EXPECT_EQ(info.Queens(), chess::util::GetBit(chess::Square::E5));
}

TEST(Position, PlayerInfo_CanCastle) {
    chess::Position::PlayerInfo<chess::Player::kWhite> info;

    info.CanCastleLong() = true;
    EXPECT_TRUE(info.CanCastleLong());
    info.CanCastleLong() = false;
    EXPECT_FALSE(info.CanCastleLong());

    info.CanCastleShort() = true;
    EXPECT_TRUE(info.CanCastleShort());
    info.CanCastleShort() = false;
    EXPECT_FALSE(info.CanCastleShort());
}

TEST(Position, PlayerInfo_DropLift) {
    chess::Position::PlayerInfo<chess::Player::kWhite> info_w;
    chess::Position::PlayerInfo<chess::Player::kBlack> info_b;

    constexpr auto pieces = std::make_tuple(chess::Piece::KING,
                                            chess::Piece::PAWN,
                                            chess::Piece::ROOK,
                                            chess::Piece::KNIGHT,
                                            chess::Piece::BISHOP,
                                            chess::Piece::QUEEN);

    info_w.Drop<std::get<0>(pieces)>(chess::Square::E1);
    info_w.Drop<std::get<1>(pieces)>(chess::Square::F7);
    info_w.Drop<std::get<2>(pieces)>(chess::Square::A2);
    info_w.Drop<std::get<3>(pieces)>(chess::Square::H8);
    info_w.Drop<std::get<4>(pieces)>(chess::Square::B6);
    info_w.Drop<std::get<5>(pieces)>(chess::Square::G4);

    EXPECT_EQ(info_w.King(),    chess::util::GetBit(chess::Square::E1));
    EXPECT_EQ(info_w.Pawns(),   chess::util::GetBit(chess::Square::F7));
    EXPECT_EQ(info_w.Rooks(),   chess::util::GetBit(chess::Square::A2));
    EXPECT_EQ(info_w.Knights(), chess::util::GetBit(chess::Square::H8));
    EXPECT_EQ(info_w.Bishops(), chess::util::GetBit(chess::Square::B6));
    EXPECT_EQ(info_w.Queens(),  chess::util::GetBit(chess::Square::G4));

    EXPECT_EQ(info_w.KingSquare(), chess::Square::E1);

    info_w.Lift<std::get<0>(pieces)>(chess::Square::E1);
    info_w.Lift<std::get<1>(pieces)>(chess::Square::F7);
    info_w.Lift<std::get<2>(pieces)>(chess::Square::A2);
    info_w.Lift<std::get<3>(pieces)>(chess::Square::H8);
    info_w.Lift<std::get<4>(pieces)>(chess::Square::B6);
    info_w.Lift<std::get<5>(pieces)>(chess::Square::G4);

    // Note lifting a king does not affect its square; a king may only
    // be dropped onto a square

    EXPECT_EQ(info_w.King(),    0);
    EXPECT_EQ(info_w.Pawns(),   0);
    EXPECT_EQ(info_w.Rooks(),   0);
    EXPECT_EQ(info_w.Knights(), 0);
    EXPECT_EQ(info_w.Bishops(), 0);
    EXPECT_EQ(info_w.Queens(),  0);

    // Testing 2nd variant of Drop()/Lift()

    info_b.Drop(std::get<0>(pieces), chess::Square::E1);
    info_b.Drop(std::get<1>(pieces), chess::Square::F7);
    info_b.Drop(std::get<2>(pieces), chess::Square::A2);
    info_b.Drop(std::get<3>(pieces), chess::Square::H8);
    info_b.Drop(std::get<4>(pieces), chess::Square::B6);
    info_b.Drop(std::get<5>(pieces), chess::Square::G4);

    EXPECT_EQ(info_b.King(),    chess::util::GetBit(chess::Square::E1));
    EXPECT_EQ(info_b.Pawns(),   chess::util::GetBit(chess::Square::F7));
    EXPECT_EQ(info_b.Rooks(),   chess::util::GetBit(chess::Square::A2));
    EXPECT_EQ(info_b.Knights(), chess::util::GetBit(chess::Square::H8));
    EXPECT_EQ(info_b.Bishops(), chess::util::GetBit(chess::Square::B6));
    EXPECT_EQ(info_b.Queens(),  chess::util::GetBit(chess::Square::G4));

    EXPECT_EQ(info_b.KingSquare(), chess::Square::E1);

    info_b.Lift(std::get<0>(pieces), chess::Square::E1);
    info_b.Lift(std::get<1>(pieces), chess::Square::F7);
    info_b.Lift(std::get<2>(pieces), chess::Square::A2);
    info_b.Lift(std::get<3>(pieces), chess::Square::H8);
    info_b.Lift(std::get<4>(pieces), chess::Square::B6);
    info_b.Lift(std::get<5>(pieces), chess::Square::G4);

    // Note lifting a king does not affect its square; a king may only
    // be dropped onto a square

    EXPECT_EQ(info_b.King(),    0);
    EXPECT_EQ(info_b.Pawns(),   0);
    EXPECT_EQ(info_b.Rooks(),   0);
    EXPECT_EQ(info_b.Knights(), 0);
    EXPECT_EQ(info_b.Bishops(), 0);
    EXPECT_EQ(info_b.Queens(),  0);
}

TEST(Position, PlayerInfo_KingSquare) {
    chess::Position::PlayerInfo<chess::Player::kWhite> info;

    info.Drop(chess::Piece::KING, chess::Square::E1);
    
    EXPECT_EQ(info.KingSquare(), chess::Square::E1);

    info.Drop(chess::Piece::KING, chess::Square::E8);

    EXPECT_EQ(info.KingSquare(), chess::Square::E8);
}

TEST(Position, PlayerInfo_Occupied) {
    chess::Position::PlayerInfo<chess::Player::kWhite> info;

    info.Drop(chess::Piece::KING,   chess::Square::E1);
    info.Drop(chess::Piece::PAWN,   chess::Square::F7);
    info.Drop(chess::Piece::ROOK,   chess::Square::A2);
    info.Drop(chess::Piece::KNIGHT, chess::Square::H8);
    info.Drop(chess::Piece::BISHOP, chess::Square::B6);
    info.Drop(chess::Piece::QUEEN,  chess::Square::G4);

    constexpr auto expect =
        (std::uint64_t(1) << chess::Square::E1) |
        (std::uint64_t(1) << chess::Square::F7) |
        (std::uint64_t(1) << chess::Square::A2) |
        (std::uint64_t(1) << chess::Square::H8) |
        (std::uint64_t(1) << chess::Square::B6) |
        (std::uint64_t(1) << chess::Square::G4);

    EXPECT_EQ(info.Occupied(), expect);

    info.Lift(chess::Piece::KING,   chess::Square::E1);
    info.Lift(chess::Piece::PAWN,   chess::Square::F7);
    info.Lift(chess::Piece::ROOK,   chess::Square::A2);
    info.Lift(chess::Piece::KNIGHT, chess::Square::H8);
    info.Lift(chess::Piece::BISHOP, chess::Square::B6);
    info.Lift(chess::Piece::QUEEN,  chess::Square::G4);

    EXPECT_EQ(info.Occupied(), 0);
}

TEST(Position, EnPassantTarget) {
    chess::Position pos;
    EXPECT_EQ(pos.EnPassantTarget(), chess::Square::Overflow);

    const std::string fen =
        "r1bqk1nr/ppp2ppp/2nbp3/3p4/2PP1N2/4P3/PP2BPPP/RNBQK2R b KQkq d3 2 7";

    ASSERT_EQ(pos.Reset(fen), chess::Position::FenError::kSuccess);

    EXPECT_EQ(pos.EnPassantTarget(), chess::Square::D3);
}

TEST(Position, FullMoveNumber) {
    chess::Position pos;
    EXPECT_EQ(pos.FullMoveNumber(), 0);

    const std::string fen =
        "r1bqk1nr/ppp2ppp/2nbp3/3p4/2PP1N2/4P3/PP2BPPP/RNBQK2R b KQkq d3 2 7";

    ASSERT_EQ(pos.Reset(fen), chess::Position::FenError::kSuccess);

    EXPECT_EQ(pos.FullMoveNumber(), 7);
}

TEST(Position, GetFen) {
    chess::Position pos;
    const std::string fen =
        "r1bqk1nr/ppp2ppp/2nbp3/3p4/2PP1N2/4P3/PP2BPPP/RNBQK2R b KQkq d3 2 7";

    ASSERT_EQ(pos.Reset(fen), chess::Position::FenError::kSuccess);

    std::ostringstream os;
    pos.Display(os);

    EXPECT_EQ(pos.GetFen(), fen) << os.str() << std::endl;

    // Starting position

    const std::string init_fen =
        "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

    ASSERT_EQ(pos.Reset(init_fen), chess::Position::FenError::kSuccess);

    os.str(""); os.clear();
    pos.Display(os);

    EXPECT_EQ(pos.GetFen(), init_fen) << os.str() << std::endl;
}

TEST(Position, HalfMoveNumber) {
    chess::Position pos;
    EXPECT_EQ(pos.HalfMoveNumber(), 0);

    const std::string fen =
        "r1bqk1nr/ppp2ppp/2nbp3/3p4/2PP1N2/4P3/PP2BPPP/RNBQK2R b KQkq d3 2 7";

    ASSERT_EQ(pos.Reset(fen), chess::Position::FenError::kSuccess);

    EXPECT_EQ(pos.HalfMoveNumber(), 2);
}

TEST(Position, Reset) {
    auto pos = chess::Position();
    EXPECT_EQ(pos.Reset(), chess::Position::FenError::kSuccess);

    std::vector<std::string> nominal_tests = {
        "rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR     b KQkq e3 0 1",
        "rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR     b KQkq E3 0 1",
        "rnbqkbnr/pp1ppppp/8/2p5/4P3/8/PPPP1PPP/RNBQKBNR   w KQkq c6 0 2",
        "rnbqkbnr/pp1ppppp/8/2p5/4P3/5N2/PPPP1PPP/RNBQKB1R b KQkq -  1 2",
        "rnbqkbnr/pp1ppppp/8/2p5/4P3/5N2/PPPP1PPP/RNBQKB1R b            ",
        "rnbqkbnr/pp1ppppp/8/2p5/4P3/5N2/PPPP1PPP/RNBQKB1R B            ",
        "rnbqkbnr/pp1ppppp/8/2p5/4P3/5N2/PPPP1PPP/RNBQKB1R b KQkq       ",
        "rnbqkbnr/pp1ppppp/8/2p5/4P3/5N2/PPPP1PPP/RNBQKB1R b -          ",
        "rnbqkbnr/pp1ppppp/8/2p5/4P3/5N2/PPPP1PPP/RNBQKB1R b -    -     ",
        "rnbqkbnr/pp1ppppp/8/2p5/4P3/5N2/PPPP1PPP/RNBQKB1R b -    -  1 2",
        "rnbqkbnr/pp1ppppp/8/2p5/4P3/5N2/PPPP1PPP/RNBQKB1R b KQkq -     ",
        "rnbqkbnr/pp1ppppp/8/2p5/4P3/5N2/PPPP1PPP/RNBQKB1R b KQkq -  1  ",
        "rnbqkbnr/pp1ppppp/8/2p5/4P3/8/PPPP1PPP/RNBQKBNR   w KQkq c6    ",
        "rnbqkbnr/pp1ppppp/8/2p5/4P3/8/PPPP1PPP/RNBQKBNR   w KQkq c6 0  "
    };

    for (const std::string& fen : nominal_tests) {
        const auto code = pos.Reset(fen);
        EXPECT_EQ(code, chess::Position::FenError::kSuccess)
            << "\t" << fen << ": " << chess::Position::ErrorToString(code)
            << std::endl;
    }

    std::map<std::string, chess::Position::FenError> tests = {
        {"rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/1/RNBQKBNR   b KQkq e3  0  1",
            chess::Position::FenError::kNumberOfRanks},
        {"rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/*NBQKBNR     b KQkq e3  0  1",
            chess::Position::FenError::kInvalidCharacter},
        {"rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR1    b KQkq e3  0  1",
            chess::Position::FenError::kSizeOfRank},
        {"rnbqkbnr/pppppppp/8/8/4P3/8/PPPP2PPP/RNBQKBNR     b KQkq e3  0  1",
            chess::Position::FenError::kSizeOfRank},
        {"rnbqkbnr/pppppppp/8/8/4P3/8/PPPP0PPP/RNBQKBNR     b KQkq e3  0  1",
            chess::Position::FenError::kSizeOfRank},
        {"rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR     b KQkq e3  0  *",
            chess::Position::FenError::kFullMoveNumber},
        {"rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR     b KQkq e3  0 -1",
            chess::Position::FenError::kFullMoveNumber},
        {"rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR     b KQkq e3  -  1",
            chess::Position::FenError::kHalfMoveClock},
        {"rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR     b KQkq e3  *  1",
            chess::Position::FenError::kHalfMoveClock},
        {"rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR     b KQkq e3 -1  1",
            chess::Position::FenError::kHalfMoveClock},
        {"rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR     b KQkq 3e  0  1",
            chess::Position::FenError::kEnPassantSquare},
        {"rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR     b KQkq *   0  1",
            chess::Position::FenError::kEnPassantSquare},
        {"rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR     b KQkq e6  0  1",
            chess::Position::FenError::kEnPassantSquare},
        {"rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR     w KQkq e3  0  1",
            chess::Position::FenError::kEnPassantSquare},
        {"rnbqkbnr/pp1ppppp/2p5/2p5/4P3/8/PPPP1PPP/RNBQKBNR w KQkq c6  0  2",
            chess::Position::FenError::kEnPassantSquare},
        {"rnbqkbnr/pppppppp/8/8/4P3/4p3/PPPP1PPP/RNBQKBNR   b KQkq e3  0  1",
            chess::Position::FenError::kEnPassantSquare},
        {"rnbqkbnr/pppppppp/8/8/4P3/4p3/PPPP1PPP/RNBQKBNR   b K-kq e3  0  1",
            chess::Position::FenError::kCastlingRights},
        {"rnbqkbnr/pppppppp/8/8/4P3/4p3/PPPP1PPP/RNBQKBNR   p",
            chess::Position::FenError::kInvalidColor},
        {"rnbqkbnr/pppppppp/8/8/4P3/4p3/PPPP1PPP/RNBQKBNR",
            chess::Position::FenError::kMissingColor}
    };

    for (auto iter = tests.begin(), end = tests.end(); iter != end; ++iter) {
        const auto code = pos.Reset(iter->first);
        EXPECT_EQ(code, iter->second)
            << "\t" << iter->first << ": "
            << chess::Position::ErrorToString(code)
            << std::endl;
    }

    // Verify Position members are correctly filled

    ASSERT_EQ(pos.Reset(
        "r1bqk1nr/ppp2ppp/2nbp3/3p4/2PP1N2/4P3/PP2BPPP/RNBQK2R b KQkq c3 2 7"),
        chess::Position::FenError::kSuccess);

    auto white = pos.GetPlayerInfo<chess::Player::kWhite>();
    auto black = pos.GetPlayerInfo<chess::Player::kBlack>();

    const auto knights_b =
        jfern::bitops::create_mask<std::uint64_t, chess::Square::C6,
                                                  chess::Square::G8>();
    const auto knights_w =
        jfern::bitops::create_mask<std::uint64_t, chess::Square::B1,
                                                  chess::Square::F4>();
    const auto bishops_b =
        jfern::bitops::create_mask<std::uint64_t, chess::Square::D6,
                                                  chess::Square::C8>();
    const auto bishops_w =
        jfern::bitops::create_mask<std::uint64_t, chess::Square::C1,
                                                  chess::Square::E2>();
    const auto rooks_w =
        jfern::bitops::create_mask<std::uint64_t, chess::Square::A1,
                                                  chess::Square::H1>();
    const auto rooks_b =
        jfern::bitops::create_mask<std::uint64_t, chess::Square::A8,
                                                  chess::Square::H8>();
    const auto queens_b =
        jfern::bitops::create_mask<std::uint64_t, chess::Square::D8>();
    const auto queens_w =
        jfern::bitops::create_mask<std::uint64_t, chess::Square::D1>();
    const auto king_b =
        jfern::bitops::create_mask<std::uint64_t, chess::Square::E8>();
    const auto king_w =
        jfern::bitops::create_mask<std::uint64_t, chess::Square::E1>();
    const auto pawns_w =
        jfern::bitops::create_mask<std::uint64_t, chess::Square::A2,
                                                  chess::Square::B2,
                                                  chess::Square::C4,
                                                  chess::Square::D4,
                                                  chess::Square::E3,
                                                  chess::Square::F2,
                                                  chess::Square::G2,
                                                  chess::Square::H2>();
    const auto pawns_b =
        jfern::bitops::create_mask<std::uint64_t, chess::Square::A7,
                                                  chess::Square::B7,
                                                  chess::Square::C7,
                                                  chess::Square::D5,
                                                  chess::Square::E6,
                                                  chess::Square::F7,
                                                  chess::Square::G7,
                                                  chess::Square::H7>();

    EXPECT_EQ(black.Rooks(),   rooks_b);
    EXPECT_EQ(black.Pawns(),   pawns_b);
    EXPECT_EQ(black.Knights(), knights_b);
    EXPECT_EQ(black.Bishops(), bishops_b);
    EXPECT_EQ(black.Queens(),  queens_b);
    EXPECT_EQ(black.King(),    king_b);

    EXPECT_EQ(white.Rooks(),   rooks_w);
    EXPECT_EQ(white.Pawns(),   pawns_w);
    EXPECT_EQ(white.Knights(), knights_w);
    EXPECT_EQ(white.Bishops(), bishops_w);
    EXPECT_EQ(white.Queens(),  queens_w);
    EXPECT_EQ(white.King(),    king_w);

    EXPECT_TRUE(white.CanCastleShort());
    EXPECT_TRUE(white.CanCastleLong());
    EXPECT_TRUE(black.CanCastleShort());
    EXPECT_TRUE(black.CanCastleLong());

    EXPECT_EQ(white.KingSquare(), chess::Square::E1);
    EXPECT_EQ(black.KingSquare(), chess::Square::E8);

    const auto occupied_w = rooks_w   |
                            pawns_w   |
                            bishops_w |
                            knights_w |
                            queens_w  |
                            king_w;

    const auto occupied_b = rooks_b   |
                            pawns_b   |
                            bishops_b |
                            knights_b |
                            queens_b  |
                            king_b;

    EXPECT_EQ(white.Occupied(), occupied_w);
    EXPECT_EQ(black.Occupied(), occupied_b);

    EXPECT_EQ(pos.ToMove(), chess::Player::kBlack);
    EXPECT_EQ(pos.EnPassantTarget(), chess::Square::C3);
    EXPECT_EQ(pos.HalfMoveNumber(), 2);
    EXPECT_EQ(pos.FullMoveNumber(), 7);
}

TEST(Position, UnderAttack) {
    auto pos = chess::Position();

    // Under attack by a pawn or king
    ASSERT_EQ(pos.Reset("8/6k1/p5pp/8/8/P5PP/1K6/8 w - - 0 1"),
              chess::Position::FenError::kSuccess);

    std::vector<chess::Square> attacked_by_white = { chess::Square::B4,
                                                     chess::Square::F4,
                                                     chess::Square::G4,
                                                     chess::Square::H4,
                                                     chess::Square::A1,
                                                     chess::Square::A2,
                                                     chess::Square::A3,
                                                     chess::Square::B1,
                                                     chess::Square::B3,
                                                     chess::Square::C1,
                                                     chess::Square::C2,
                                                     chess::Square::C3 };

    std::vector<chess::Square> attacked_by_black = { chess::Square::B5,
                                                     chess::Square::F5,
                                                     chess::Square::G5,
                                                     chess::Square::H5,
                                                     chess::Square::F6,
                                                     chess::Square::G6,
                                                     chess::Square::H6,
                                                     chess::Square::F7,
                                                     chess::Square::H7,
                                                     chess::Square::F8,
                                                     chess::Square::G8,
                                                     chess::Square::H8 };

    auto white_attacks = [&](chess::Square square) {
        auto end = std::end(attacked_by_white);
        return std::find(std::begin(attacked_by_white), end, square) != end;
    };

    auto black_attacks = [&](chess::Square square) {
        auto end = std::end(attacked_by_black);
        return std::find(std::begin(attacked_by_black), end, square) != end;
    };

    auto run_checks = [&]() {
        for (auto square = chess::Square::H1; square <= chess::Square::A8;
             square++) {
            if (white_attacks(square)) {
                EXPECT_TRUE(pos.UnderAttack< chess::Player::kWhite>(square))
                    << chess::kSquareStr[square] << std::endl;
            } else if (black_attacks(square)) {
                EXPECT_TRUE(pos.UnderAttack< chess::Player::kBlack>(square))
                    << chess::kSquareStr[square] << std::endl;
            } else {
                EXPECT_FALSE(pos.UnderAttack<chess::Player::kWhite>(square))
                    << chess::kSquareStr[square] << std::endl;
                EXPECT_FALSE(pos.UnderAttack<chess::Player::kBlack>(square))
                    << chess::kSquareStr[square] << std::endl;
            }
        }
    };

    // Under attack by a knight
    ASSERT_EQ(pos.Reset("k4nnn/7n/7n/4n3/4N3/N7/N7/NNN4K w - - 0 1"),
              chess::Position::FenError::kSuccess);

    attacked_by_white = { chess::Square::G1,
                          chess::Square::G2,
                          chess::Square::H2,
                          chess::Square::B3,
                          chess::Square::D3,
                          chess::Square::E2,
                          chess::Square::A3,
                          chess::Square::C3,
                          chess::Square::D2,
                          chess::Square::C2,
                          chess::Square::B4,
                          chess::Square::C1,
                          chess::Square::B1,
                          chess::Square::C4,
                          chess::Square::B5,
                          chess::Square::A2,
                          chess::Square::F2,
                          chess::Square::G3,
                          chess::Square::C5,
                          chess::Square::G5,
                          chess::Square::D6,
                          chess::Square::F6 };

    attacked_by_black = { chess::Square::A7,
                          chess::Square::B7,
                          chess::Square::B8,
                          chess::Square::D7,
                          chess::Square::H7,
                          chess::Square::E6,
                          chess::Square::G6,
                          chess::Square::E7,
                          chess::Square::F6,
                          chess::Square::H6,
                          chess::Square::F7,
                          chess::Square::F8,
                          chess::Square::G5,
                          chess::Square::G8,
                          chess::Square::F5,
                          chess::Square::G4,
                          chess::Square::D3,
                          chess::Square::F3,
                          chess::Square::C4,
                          chess::Square::C6 };

    run_checks();

    // Under attack by a sliding piece
    ASSERT_EQ(pos.Reset("k7/1p1pr1pq/b3p1pp/1p6/8/PP1PP3/QPB1RP2/7K w - -"),
              chess::Position::FenError::kSuccess);

    attacked_by_white = { chess::Square::A4,
                          chess::Square::B4,
                          chess::Square::C4,
                          chess::Square::D4,
                          chess::Square::E4,
                          chess::Square::F4,
                          chess::Square::C3,
                          chess::Square::E3,
                          chess::Square::G3,
                          chess::Square::A1,
                          chess::Square::A3,
                          chess::Square::B1,
                          chess::Square::B2,
                          chess::Square::B3,
                          chess::Square::D1,
                          chess::Square::D3,
                          chess::Square::D2,
                          chess::Square::C2,
                          chess::Square::E1,
                          chess::Square::F2,
                          chess::Square::G1,
                          chess::Square::G2,
                          chess::Square::H2 };

    attacked_by_black = { chess::Square::A6,
                          chess::Square::C6,
                          chess::Square::A4,
                          chess::Square::C4,
                          chess::Square::E6,
                          chess::Square::D5,
                          chess::Square::F5,
                          chess::Square::F6,
                          chess::Square::H5,
                          chess::Square::G5,
                          chess::Square::A7,
                          chess::Square::B7,
                          chess::Square::B8,
                          chess::Square::B5,
                          chess::Square::D7,
                          chess::Square::F7,
                          chess::Square::G7,
                          chess::Square::E8,
                          chess::Square::H8,
                          chess::Square::H6,
                          chess::Square::G8,
                          chess::Square::G6 };

    run_checks();
}

TEST(Position, Validate) {
    auto pos = chess::Position();

    std::map<std::string, chess::Position::FenError> tests = {
        {"6k1/8/8/8/8/8/8/1p4K1                   w -    - 0 1",
            chess::Position::FenError::kPawnsOnBackRank},
        {"6k1/8/8/8/8/8/8/1P4K1                   w -    - 0 1",
            chess::Position::FenError::kPawnsOnBackRank},
        {"p5k1/8/8/8/8/8/8/6K1                    w -    - 0 1",
            chess::Position::FenError::kPawnsOnBackRank},
        {"P5k1/8/8/8/8/8/8/6K1                    w -    - 0 1",
            chess::Position::FenError::kPawnsOnBackRank},
        {"6k1/8/4k3/8/8/8/8/6K1                   w -    - 0 1",
            chess::Position::FenError::kNumberOfKings},
        {"6k1/8/4K3/8/8/8/8/6K1                   w -    - 0 1",
            chess::Position::FenError::kNumberOfKings},
        {"6k1/8/8/8/8/8/5p2/6K1                   b -    - 0 1",
            chess::Position::FenError::kKingCanBeCaptured},
        {"6k1/b7/8/8/8/8/8/6K1                    b -    - 0 1",
            chess::Position::FenError::kKingCanBeCaptured},
        {"6k1/8/8/8/8/8/8/3r2K1                   b -    - 0 1",
            chess::Position::FenError::kKingCanBeCaptured},
        {"6k1/8/8/8/8/7n/8/6K1                    b -    - 0 1",
            chess::Position::FenError::kKingCanBeCaptured},
        {"6k1/8/8/8/8/4q3/8/6K1                   b -    - 0 1",
            chess::Position::FenError::kKingCanBeCaptured},
        {"6kK/8/8/8/8/8/8/8                       b -    - 0 1",
            chess::Position::FenError::kKingCanBeCaptured},
        {"6kK/8/8/8/8/8/8/8                       w -    - 0 1",
            chess::Position::FenError::kKingCanBeCaptured},
        {"r5kr/8/8/8/8/8/8/R5KR                   w K    - 0 1",
            chess::Position::FenError::kWhiteMayNotCastle},
        {"r5kr/8/8/8/8/8/8/R5KR                   w Q    - 0 1",
            chess::Position::FenError::kWhiteMayNotCastle},
        {"r5kr/8/8/8/8/8/8/R5KR                   w k    - 0 1",
            chess::Position::FenError::kBlackMayNotCastle},
        {"r5kr/8/8/8/8/8/8/R5KR                   w q    - 0 1",
            chess::Position::FenError::kBlackMayNotCastle},
        {"r3k2r/8/8/8/8/8/8/4K2R                  w KQkq - 0 1",
            chess::Position::FenError::kWhiteMayNotCastleLong},
        {"r3k2r/8/8/8/8/8/8/R3K3                  w KQkq - 0 1",
            chess::Position::FenError::kWhiteMayNotCastleShort},
        {"4k2r/8/8/8/8/8/8/R3K2R                  w KQkq - 0 1",
            chess::Position::FenError::kBlackMayNotCastleLong},
        {"r3k3/8/8/8/8/8/8/R3K2R                  w KQkq - 0 1",
            chess::Position::FenError::kBlackMayNotCastleShort},
        {"r3k2r/pp5p/2p3p1/3p1p2/4pp2/8/8/R3K2R   w -    - 0 1",
            chess::Position::FenError::kTooManyPawns},
        {"r3k2r/nn5n/2n3nn/3nnn2/4nn2/8/8/R3K2R   w -    - 0 1",
            chess::Position::FenError::kTooManyKnights},
        {"r3k2r/bb5b/2b3b1/3bbb2/3bbb2/8/8/R3K2R  w -    - 0 1",
            chess::Position::FenError::kTooManyBishops},
        {"r3k2r/rr5r/2r3r1/3r1r2/4rr2/8/8/R3K2R   w -    - 0 1",
            chess::Position::FenError::kTooManyRooks},
        {"r3k2r/qq5q/2q3q1/3qqq2/3qqq2/8/8/R3K2R  w -    - 0 1",
            chess::Position::FenError::kTooManyQueens},
        {"r3k2r/8/8/8/2P1P2P/1P1P1P1P/P5P1/R3K2R  w -    - 0 1",
            chess::Position::FenError::kTooManyPawns},
        {"r3k2r/8/8/8/2NNN2N/NN1N1N1N/N5N1/R3K2R  w -    - 0 1",
            chess::Position::FenError::kTooManyKnights},
        {"r3k2r/8/8/8/2BBBB1B/1B1B1B1B/B5B1/R3K2R w -    - 0 1",
            chess::Position::FenError::kTooManyBishops},
        {"r3k2r/8/8/8/2R1R2R/1R1R1R1R/R5R1/R3K2R  b -    - 0 1",
            chess::Position::FenError::kTooManyRooks},
        {"r3k2r/8/8/8/2Q1Q2Q/1QQQQQ1Q/Q5Q1/R3K2R  b -    - 0 1",
            chess::Position::FenError::kTooManyQueens}
    };

    for (auto iter = tests.begin(), end = tests.end(); iter != end; ++iter) {
        const auto code = pos.Reset(iter->first);
        EXPECT_EQ(code, iter->second)
            << "\t" << iter->first << ": "
            << chess::Position::ErrorToString(code)
            << std::endl;
    }
}

}  // namespace
