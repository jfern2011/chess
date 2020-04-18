/**
 *  \file   data_tables_ut.cc
 *  \author Jason Fernandez
 *  \date   04/02/2020
 */

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <vector>

#include "gtest/gtest.h"
#include "chess/data_tables.h"
#include "chess/debug.h"
#include "chess/util.h"

namespace {

TEST(data_tables, k3rdRank) {
    const std::uint64_t rank3_white = 0x000000ff0000;
    const std::uint64_t rank3_black = 0xff0000000000;

    EXPECT_EQ(rank3_white, chess::data_tables::k3rdRank<chess::Player::kWhite>);
    EXPECT_EQ(rank3_black, chess::data_tables::k3rdRank<chess::Player::kBlack>);
}

TEST(data_tables, kA1H8_64) {

    std::vector<chess::Square> diag_h1 = { chess::Square::H1 };

    std::vector<chess::Square> diag_g1 = { chess::Square::G1,
                                           chess::Square::H2 };

    std::vector<chess::Square> diag_f1 = { chess::Square::F1,
                                           chess::Square::G2,
                                           chess::Square::H3 };

    std::vector<chess::Square> diag_e1 = { chess::Square::E1,
                                           chess::Square::F2,
                                           chess::Square::G3,
                                           chess::Square::H4 };

    std::vector<chess::Square> diag_d1 = { chess::Square::D1,
                                           chess::Square::E2,
                                           chess::Square::F3,
                                           chess::Square::G4,
                                           chess::Square::H5 };

    std::vector<chess::Square> diag_c1 = { chess::Square::C1,
                                           chess::Square::D2,
                                           chess::Square::E3,
                                           chess::Square::F4,
                                           chess::Square::G5,
                                           chess::Square::H6 };

    std::vector<chess::Square> diag_b1 = { chess::Square::B1,
                                           chess::Square::C2,
                                           chess::Square::D3,
                                           chess::Square::E4,
                                           chess::Square::F5,
                                           chess::Square::G6,
                                           chess::Square::H7 };

    std::vector<chess::Square> diag_a1 = { chess::Square::A1,
                                           chess::Square::B2,
                                           chess::Square::C3,
                                           chess::Square::D4,
                                           chess::Square::E5,
                                           chess::Square::F6,
                                           chess::Square::G7,
                                           chess::Square::H8 };

    std::vector<chess::Square> diag_a2 = { chess::Square::A2,
                                           chess::Square::B3,
                                           chess::Square::C4,
                                           chess::Square::D5,
                                           chess::Square::E6,
                                           chess::Square::F7,
                                           chess::Square::G8 };

    std::vector<chess::Square> diag_a3 = { chess::Square::A3,
                                           chess::Square::B4,
                                           chess::Square::C5,
                                           chess::Square::D6,
                                           chess::Square::E7,
                                           chess::Square::F8 };

    std::vector<chess::Square> diag_a4 = { chess::Square::A4,
                                           chess::Square::B5,
                                           chess::Square::C6,
                                           chess::Square::D7,
                                           chess::Square::E8 };

    std::vector<chess::Square> diag_a5 = { chess::Square::A5,
                                           chess::Square::B6,
                                           chess::Square::C7,
                                           chess::Square::D8 };

    std::vector<chess::Square> diag_a6 = { chess::Square::A6,
                                           chess::Square::B7,
                                           chess::Square::C8 };

    std::vector<chess::Square> diag_a7 = { chess::Square::A7,
                                           chess::Square::B8 };

    std::vector<chess::Square> diag_a8 = { chess::Square::A8 };

    std::vector<std::vector<chess::Square>> diags = {
        diag_h1,
        diag_g1,
        diag_f1,
        diag_e1,
        diag_d1,
        diag_c1,
        diag_b1,
        diag_a1,
        diag_a2,
        diag_a3,
        diag_a4,
        diag_a5,
        diag_a6,
        diag_a7,
        diag_a8
    };

    // Create the expected 64-bit diagonal
    auto createDiag = [&diags](chess::Square square) {
        std::uint64_t diag64 = 0;
        for (const auto& diag : diags) {
            auto iter = std::find(diag.begin(), diag.end(), square);
            if (iter != diag.end()) {
                for (auto diagSquare : diag) {
                    diag64 |= std::uint64_t(1) << diagSquare;
                }
                break;
            }
        }

        // If diag is zero, 'square' was invalid
        return diag64;
    };

    for (auto i = chess::Square::H1; i <= chess::Square::A8;
         i = static_cast<chess::Square>(i+1)) {
        const std::uint64_t diag64 = createDiag(i);

        EXPECT_EQ(diag64, chess::data_tables::kA1H8_64[i]);
    }
}

TEST(data_tables, kBackRank) {
    const std::uint64_t back_rank_white = 0x00000000000000ff;
    const std::uint64_t back_rank_black = 0xff00000000000000;

    EXPECT_EQ(back_rank_white,
              chess::data_tables::kBackRank<chess::Player::kWhite>);
    EXPECT_EQ(back_rank_black,
              chess::data_tables::kBackRank<chess::Player::kBlack>);
}

TEST(data_tables, bishop_attacks) {
    const int diag_size_a1h8[] = {
        1, 2, 3, 4, 5, 6, 7, 8,
        2, 3, 4, 5, 6, 7, 8, 7,
        3, 4, 5, 6, 7, 8, 7, 6,
        4, 5, 6, 7, 8, 7, 6, 5,
        5, 6, 7, 8, 7, 6, 5, 4,
        6, 7, 8, 7, 6, 5, 4, 3,
        7, 8, 7, 6, 5, 4, 3, 2,
        8, 7, 6, 5, 4, 3, 2, 1
    };

    const int diag_size_h1a8[] = {
        8, 7, 6, 5, 4, 3, 2, 1,
        7, 8, 7, 6, 5, 4, 3, 2,
        6, 7, 8, 7, 6, 5, 4, 3,
        5, 6, 7, 8, 7, 6, 5, 4,
        4, 5, 6, 7, 8, 7, 6, 5,
        3, 4, 5, 6, 7, 8, 7, 6,
        2, 3, 4, 5, 6, 7, 8, 7,
        1, 2, 3, 4, 5, 6, 7, 8
    };

    const int diag_shift_45r[] = {
         0,  1,  3,  6, 10, 15, 21, 28,
         1,  3,  6, 10, 15, 21, 28, 36,
         3,  6, 10, 15, 21, 28, 36, 43,
         6, 10, 15, 21, 28, 36, 43, 49,
        10, 15, 21, 28, 36, 43, 49, 54,
        15, 21, 28, 36, 43, 49, 54, 58,
        21, 28, 36, 43, 49, 54, 58, 61,
        28, 36, 43, 49, 54, 58, 61, 63
    };

    const int diag_shift_45l[] = {
        28, 21, 15, 10,  6,  3,  1,  0,
        36, 28, 21, 15, 10,  6,  3,  1,
        43, 36, 28, 21, 15, 10,  6,  3,
        49, 43, 36, 28, 21, 15, 10,  6,
        54, 49, 43, 36, 28, 21, 15, 10,
        58, 54, 49, 43, 36, 28, 21, 15,
        61, 58, 54, 49, 43, 36, 28, 21,
        63, 61, 58, 54, 49, 43, 36, 28
    };

    const chess::Square rotate45r[] = {
        chess::Square::H1,
        chess::Square::H2,
            chess::Square::G1,
        chess::Square::H3,
            chess::Square::G2,
                chess::Square::F1,
        chess::Square::H4,
            chess::Square::G3,
                chess::Square::F2,
                    chess::Square::E1,
        chess::Square::H5,
            chess::Square::G4,
                chess::Square::F3,
                    chess::Square::E2,
                        chess::Square::D1,
        chess::Square::H6,
            chess::Square::G5,
                chess::Square::F4,
                    chess::Square::E3,
                        chess::Square::D2,
                            chess::Square::C1,
        chess::Square::H7,
            chess::Square::G6,
                chess::Square::F5,
                    chess::Square::E4,
                        chess::Square::D3,
                            chess::Square::C2,
                                chess::Square::B1,
        chess::Square::H8,
            chess::Square::G7,
                chess::Square::F6,
                    chess::Square::E5,
                        chess::Square::D4,
                            chess::Square::C3,
                                chess::Square::B2,
                                    chess::Square::A1,
        chess::Square::G8,
            chess::Square::F7,
                chess::Square::E6,
                    chess::Square::D5,
                        chess::Square::C4,
                            chess::Square::B3,
                                chess::Square::A2,
        chess::Square::F8,
            chess::Square::E7,
                chess::Square::D6,
                    chess::Square::C5,
                        chess::Square::B4,
                            chess::Square::A3,
        chess::Square::E8,
            chess::Square::D7,
                chess::Square::C6,
                    chess::Square::B5,
                        chess::Square::A4,
        chess::Square::D8,
            chess::Square::C7,
                chess::Square::B6,
                    chess::Square::A5,
        chess::Square::C8,
            chess::Square::B7,
                chess::Square::A6,
        chess::Square::B8,
            chess::Square::A7,
        chess::Square::A8
    };

    const chess::Square rotate45l[] = {
        chess::Square::A1,
        chess::Square::B1,
            chess::Square::A2,
        chess::Square::C1,
            chess::Square::B2,
                chess::Square::A3,
        chess::Square::D1,
            chess::Square::C2,
                chess::Square::B3,
                    chess::Square::A4,
        chess::Square::E1,
            chess::Square::D2,
                chess::Square::C3,
                    chess::Square::B4,
                        chess::Square::A5,
        chess::Square::F1,
            chess::Square::E2,
                chess::Square::D3,
                    chess::Square::C4,
                        chess::Square::B5,
                            chess::Square::A6,
        chess::Square::G1,
            chess::Square::F2,
                chess::Square::E3,
                    chess::Square::D4,
                        chess::Square::C5,
                            chess::Square::B6,
                                chess::Square::A7,
        chess::Square::H1,
            chess::Square::G2,
                chess::Square::F3,
                    chess::Square::E4,
                        chess::Square::D5,
                            chess::Square::C6,
                                chess::Square::B7,
                                    chess::Square::A8,
        chess::Square::H2,
            chess::Square::G3,
                chess::Square::F4,
                    chess::Square::E5,
                        chess::Square::D6,
                            chess::Square::C7,
                                chess::Square::B8,
        chess::Square::H3,
            chess::Square::G4,
                chess::Square::F5,
                    chess::Square::E6,
                        chess::Square::D7,
                            chess::Square::C8,
        chess::Square::H4,
            chess::Square::G5,
                chess::Square::F6,
                    chess::Square::E7,
                        chess::Square::D8,
        chess::Square::H5,
            chess::Square::G6,
                chess::Square::F7,
                    chess::Square::E8,
        chess::Square::H6,
            chess::Square::G7,
                chess::Square::F8,
        chess::Square::H7,
            chess::Square::G8,
        chess::Square::H8
    };

    auto gen_a1h8_occupancies = [&](chess::Square square) {
        const int diag_size = diag_size_a1h8[square];

        std::vector<std::uint64_t> occupancies;
        for (int mask = 0; mask < (1 << diag_size); mask++) {

            // First, produce a mask in the 45-degree clockwise-rotated frame:
            auto mask64R = std::uint64_t(mask) << diag_shift_45r[square];

            // Now, rotate back to the unrotated frame
            std::uint64_t mask64 = 0;
            while (mask64R) {
                const std::int8_t bitIndex = chess::util::GetLsb(mask64R);
                mask64 |= std::uint64_t(1) << rotate45r[bitIndex];
                chess::util::ClearBit(bitIndex, &mask64R);
            }
            occupancies.push_back(mask64);
        }
        return occupancies;
    };

    auto gen_h1a8_occupancies = [&](chess::Square square) {
        const int diag_size = diag_size_h1a8[square];

        std::vector<std::uint64_t> occupancies;
        for (int mask = 0; mask < (1 << diag_size); mask++) {

            // First, produce a mask in the 45-degree clockwise-rotated frame:
            auto mask64R = std::uint64_t(mask) << diag_shift_45l[square];

            // Now, rotate back to the unrotated frame
            std::uint64_t mask64 = 0;
            while (mask64R) {
                const std::int8_t bitIndex = chess::util::GetLsb(mask64R);
                mask64 |= std::uint64_t(1) << rotate45l[bitIndex];
                chess::util::ClearBit(bitIndex, &mask64R);
            }
            occupancies.push_back(mask64);
        }
        return occupancies;
    };

    auto gen_attacks_from_diag =
            [](chess::Square square, std::uint64_t occupied) {
        std::uint64_t attacks = 0;
        chess::util::ClearBit(square, &occupied);
        for (int sq = chess::util::ToIntType(square); true; sq += 7) {
            const auto bit64 = chess::util::GetBit<std::uint64_t>(sq);
            attacks |= bit64;
            if (chess::util::GetFile(sq) == 0 ||
                chess::util::GetRank(sq) == 7 ||
                (bit64 & occupied)) break;
        }

        chess::util::ClearBit(square, &occupied);
        for (int sq = chess::util::ToIntType(square); true; sq += 9) {
            const auto bit64 = chess::util::GetBit<std::uint64_t>(sq);
            attacks |= bit64;
            if (chess::util::GetFile(sq) == 7 ||
                chess::util::GetRank(sq) == 7 ||
                (bit64 & occupied)) break;
        }

        chess::util::ClearBit(square, &occupied);
        for (int sq = chess::util::ToIntType(square); true; sq -= 9) {
            const auto bit64 = chess::util::GetBit<std::uint64_t>(sq);
            attacks |= bit64;
            if (chess::util::GetFile(sq) == 0 ||
                chess::util::GetRank(sq) == 0 ||
                (bit64 & occupied)) break;
        }

        chess::util::ClearBit(square, &occupied);
        for (int sq = chess::util::ToIntType(square); true;
             sq -= 7) {
            const auto bit64 = chess::util::GetBit<std::uint64_t>(sq);
            attacks |= bit64;
            if (chess::util::GetFile(sq) == 7 ||
                chess::util::GetRank(sq) == 0 ||
                (bit64 & occupied)) break;
        }

        // The bishop doesn't attack the square it's on:
        chess::util::ClearBit(square, &attacks);

        return attacks;
    };

    for (auto square = chess::Square::H1; square <= chess::Square::A8;
         square = static_cast<chess::Square>(square+1)) {
        std::vector<std::uint64_t> occupancies_a1h8 =
            gen_a1h8_occupancies(square);
        std::vector<std::uint64_t> occupancies_h1a8 =
            gen_h1a8_occupancies(square);

        for (auto occupancy_a1h8 : occupancies_a1h8) {
            for (auto occupancy_h1a8 : occupancies_h1a8) {
                const std::uint64_t occupied = occupancy_a1h8 | occupancy_h1a8;

                // Get the squares attacked by a bishop on this square
                const std::uint64_t expected_attacks =
                    gen_attacks_from_diag(square, occupied);

                // Now, get the same thing by table lookup and compare:
                const std::uint64_t except = chess::kFileA |
                                             chess::kFileH |
                                             chess::kRank1 |
                                             chess::kRank8 |
                                             std::uint64_t(1) << square;

                const std::uint64_t occupancy = occupied ^ (occupied & except);
                const std::uint32_t index =
                    chess::data_tables::kBishopOffsets[square] +
                        ((occupancy*chess::data_tables::kDiagMagics[square]) >>
                            chess::data_tables::kBishopDbShifts[square]);

                const std::uint64_t attacks =
                    chess::data_tables::bishop_attacks[index];

                // Assert since chances are if one check fails, many others
                // will also

                ASSERT_EQ(attacks, expected_attacks)
                    << "\nOccupied[" << chess::kSquareStr[square] << "]:"
                    << chess::debug::PrintBitBoard(occupied)
                    << "Expected:"
                    << chess::debug::PrintBitBoard(expected_attacks)
                    << "Actual:"
                    << chess::debug::PrintBitBoard(
                        attacks);
            }
        }
    }
}

TEST(data_tables, printDiag) {
#if 0
    for (int i = 0; i < 64; i++) {
        auto mask = chess::data_tables::internal::NorthEastMask(i);
        std::printf("0x%016lXULL,", mask);
        if (i > 0 && i % 2 != 0) {
            std::printf("\n");
        }
    }

    std::printf("\n");

    for (int i = 0; i < 64; i++) {
        auto mask = chess::data_tables::internal::NorthWestMask(i);
        std::printf("0x%016lXULL,", mask);
        if (i > 0 && i % 2 != 0) {
            std::printf("\n");
        }
    }

    std::printf("\n");

    for (int i = 0; i < 64; i++) {
        auto mask = chess::data_tables::internal::SouthEastMask(i);
        std::printf("0x%016lXULL,", mask);
        if (i > 0 && i % 2 != 0) {
            std::printf("\n");
        }
    }

    std::printf("\n");

    for (int i = 0; i < 64; i++) {
        auto mask = chess::data_tables::internal::SouthWestMask(i);
        std::printf("0x%016lXULL,", mask);
        if (i > 0 && i % 2 != 0) {
            std::printf("\n");
        }
    }

    for (int i = 0; i < 64; i++) {
        auto shift = chess::data_tables::internal::BishopDbShift(i);
        if (i > 0 && i % 8 == 0) {
            std::printf("\n");
        }
        std::printf("%2d, ", shift);
    }

    std::printf("\n");

    for (int i = 0; i < 64; i++) {
        auto offset = chess::data_tables::internal::DiagOffset(i);
        if (i > 0 && i % 8 == 0) {
            std::printf("\n");
        }
        std::printf("%4d, ", offset);
    }

    std::printf("\n");
#endif
}

TEST(data_tables, kH1A8_64) {

    std::vector<chess::Square> diag_a1 = { chess::Square::A1 };

    std::vector<chess::Square> diag_b1 = { chess::Square::B1,
                                           chess::Square::A2 };

    std::vector<chess::Square> diag_c1 = { chess::Square::C1,
                                           chess::Square::B2,
                                           chess::Square::A3 };

    std::vector<chess::Square> diag_d1 = { chess::Square::D1,
                                           chess::Square::C2,
                                           chess::Square::B3,
                                           chess::Square::A4 };

    std::vector<chess::Square> diag_e1 = { chess::Square::E1,
                                           chess::Square::D2,
                                           chess::Square::C3,
                                           chess::Square::B4,
                                           chess::Square::A5 };

    std::vector<chess::Square> diag_f1 = { chess::Square::F1,
                                           chess::Square::E2,
                                           chess::Square::D3,
                                           chess::Square::C4,
                                           chess::Square::B5,
                                           chess::Square::A6 };

    std::vector<chess::Square> diag_g1 = { chess::Square::G1,
                                           chess::Square::F2,
                                           chess::Square::E3,
                                           chess::Square::D4,
                                           chess::Square::C5,
                                           chess::Square::B6,
                                           chess::Square::A7 };

    std::vector<chess::Square> diag_h1 = { chess::Square::H1,
                                           chess::Square::G2,
                                           chess::Square::F3,
                                           chess::Square::E4,
                                           chess::Square::D5,
                                           chess::Square::C6,
                                           chess::Square::B7,
                                           chess::Square::A8 };

    std::vector<chess::Square> diag_h2 = { chess::Square::H2,
                                           chess::Square::G3,
                                           chess::Square::F4,
                                           chess::Square::E5,
                                           chess::Square::D6,
                                           chess::Square::C7,
                                           chess::Square::B8 };

    std::vector<chess::Square> diag_h3 = { chess::Square::H3,
                                           chess::Square::G4,
                                           chess::Square::F5,
                                           chess::Square::E6,
                                           chess::Square::D7,
                                           chess::Square::C8 };

    std::vector<chess::Square> diag_h4 = { chess::Square::H4,
                                           chess::Square::G5,
                                           chess::Square::F6,
                                           chess::Square::E7,
                                           chess::Square::D8 };

    std::vector<chess::Square> diag_h5 = { chess::Square::H5,
                                           chess::Square::G6,
                                           chess::Square::F7,
                                           chess::Square::E8 };

    std::vector<chess::Square> diag_h6 = { chess::Square::H6,
                                           chess::Square::G7,
                                           chess::Square::F8 };

    std::vector<chess::Square> diag_h7 = { chess::Square::H7,
                                           chess::Square::G8 };

    std::vector<chess::Square> diag_h8 = { chess::Square::H8 };

    std::vector<std::vector<chess::Square>> diags = {
        diag_a1,
        diag_b1,
        diag_c1,
        diag_d1,
        diag_e1,
        diag_f1,
        diag_g1,
        diag_h1,
        diag_h2,
        diag_h3,
        diag_h4,
        diag_h5,
        diag_h6,
        diag_h7,
        diag_h8
    };

    // Create the expected 64-bit diagonal
    auto createDiag = [&diags](chess::Square square) {
        std::uint64_t diag64 = 0;
        for (const auto& diag : diags) {
            auto iter = std::find(diag.begin(), diag.end(), square);
            if (iter != diag.end()) {
                for (auto diagSquare : diag) {
                    diag64 |= std::uint64_t(1) << diagSquare;
                }
                break;
            }
        }

        // If diag is zero, 'square' was invalid
        return diag64;
    };

    for (auto i = chess::Square::H1; i <= chess::Square::A8;
         i = static_cast<chess::Square>(i+1)) {
        const std::uint64_t diag64 = createDiag(i);

        EXPECT_EQ(diag64, chess::data_tables::kH1A8_64[i]);
    }
}

TEST(data_tables, lsb) {
    /*
     * Bitscan forward algorithm taken from here:
     *
     * https://www.chessprogramming.org/BitScan
     */
    constexpr std::int8_t index64[64] = {
        0, 47,  1, 56, 48, 27,  2, 60,
       57, 49, 41, 37, 28, 16,  3, 61,
       54, 58, 35, 52, 50, 42, 21, 44,
       38, 32, 29, 23, 17, 11,  4, 62,
       46, 55, 26, 59, 40, 36, 15, 53,
       34, 51, 20, 43, 31, 22, 10, 45,
       25, 39, 14, 33, 19, 30,  9, 24,
       13, 18,  8, 12,  7,  6,  5, 63
    };

    auto bitscanForward = [&index64](std::uint64_t bb) {
        constexpr auto debruijn64 = std::uint64_t(0x03f79d71b4cb0a89);
        if (bb == 0) return std::int8_t(-1);
        return index64[((bb ^ (bb-1)) * debruijn64) >> 58];
    };

    ASSERT_EQ(chess::data_tables::kLsb.size(),
              std::size_t(std::numeric_limits<std::uint16_t>::max())+1);

    for (std::size_t i = 0; i < chess::data_tables::kLsb.size(); i++) {
        ASSERT_EQ(chess::data_tables::kLsb[i], bitscanForward(i));
    }
}

TEST(data_tables, msb) {
    /*
     * Bitscan reverse algorithm taken from here:
     *
     * https://www.chessprogramming.org/BitScan
     */
    constexpr std::int8_t index64[64] = {
        0, 47,  1, 56, 48, 27,  2, 60,
       57, 49, 41, 37, 28, 16,  3, 61,
       54, 58, 35, 52, 50, 42, 21, 44,
       38, 32, 29, 23, 17, 11,  4, 62,
       46, 55, 26, 59, 40, 36, 15, 53,
       34, 51, 20, 43, 31, 22, 10, 45,
       25, 39, 14, 33, 19, 30,  9, 24,
       13, 18,  8, 12,  7,  6,  5, 63
    };

    auto bitscanReverse = [&index64](std::uint64_t bb) {
        constexpr auto debruijn64 = std::uint64_t(0x03f79d71b4cb0a89);
        if (bb == 0) return std::int8_t(-1);

        bb |= bb >> 1; 
        bb |= bb >> 2;
        bb |= bb >> 4;
        bb |= bb >> 8;
        bb |= bb >> 16;
        bb |= bb >> 32;

       return index64[(bb * debruijn64) >> 58];
    };

    ASSERT_EQ(chess::data_tables::kMsb.size(),
              std::size_t(std::numeric_limits<std::uint16_t>::max())+1);

    
    for (std::size_t i = 0; i < chess::data_tables::kMsb.size(); i++) {
        ASSERT_EQ(chess::data_tables::kMsb[i], bitscanReverse(i));
    }
}

TEST(data_tables, popCnt) {
    /*
     * Algorithm taken from here:
     *
     * https://www.chessprogramming.org/Population_Count
     */
    constexpr auto k1 = std::uint64_t(0x5555555555555555); /*  -1/3   */
    constexpr auto k2 = std::uint64_t(0x3333333333333333); /*  -1/5   */
    constexpr auto k4 = std::uint64_t(0x0f0f0f0f0f0f0f0f); /*  -1/17  */
    constexpr auto kf = std::uint64_t(0x0101010101010101); /*  -1/255 */

    auto popCount = [&](std::uint64_t x) {
        /* put count of each 2 bits into those 2 bits */
        x =  x       - ((x >> 1)  & k1);

        /* put count of each 4 bits into those 4 bits */
        x = (x & k2) + ((x >> 2)  & k2);

        /* put count of each 8 bits into those 8 bits */
        x = (x       +  (x >> 4)) & k4 ;

         /* 8 most significant bits of x + (x<<8) + (x<<16) + (x<<24) + ...  */
        x = (x * kf) >> 56;

        return std::int8_t(x);
    };

    ASSERT_EQ(chess::data_tables::kPop.size(),
              std::size_t(std::numeric_limits<std::uint16_t>::max())+1);

    for (std::size_t i = 0; i < chess::data_tables::kPop.size(); i++) {
        ASSERT_EQ(chess::data_tables::kPop[i], popCount(i));
    }
}

}  // namespace
