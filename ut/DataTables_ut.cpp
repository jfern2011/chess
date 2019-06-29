#include <algorithm>
#include <map>
#include <vector>

#include "gtest/gtest.h"

/*
 * Note: Include *after* gtest to protect against
 *       name conflicts with abort macros
 */
#include "src/DataTables4.h"
#include "src/chess_util4.h"
#include "util/bit_tools.h"

namespace
{
    /**
     * Helper function to generate the different ways a
     * particular file/rank pair or pair of diagonals could
     * be occupied; the pair is determined by \a square
     *
     * @param[in]  square     Generate occupancy variations
     *                        for a piece on this square
     * @param[in]  piece      The piece (a rook or bishop)
     * @param[out] variations The possible occupancies
     */
    void get_occupancy_variations(int square, Chess::piece_t piece,
        std::vector<Chess::uint64>& variations)
    {
        variations.clear();

        const auto& tables = Chess::DataTables::get();
        const Chess::uint64 occupancy =
            piece == Chess::piece_t::rook ? 
                tables.rook_attacks_mask  [square] :
                tables.bishop_attacks_mask[square] ;

        const Chess::uint64 one = 1;

        const int n_bits = Util::bit_count(occupancy);

        std::vector<int> i_to_occ(one << n_bits);

        int bit_index = 0;
        for ( Chess::uint64 temp = occupancy ; temp; )
        {
            const int lsb = Util::get_lsb(temp);
            Util::clear_bit(lsb, temp);

            i_to_occ[ bit_index++ ] = lsb;
        }

        for (Chess::uint64 i = 0; i < (one << n_bits); i++)
        {
            std::vector<int> indexes;
            Util::get_set_bits( i, indexes );

            Chess::uint64 variation = 0;
            for (int index : indexes)
            {
                variation |=
                    one << i_to_occ[ index ];
            }

            variations.push_back(
                variation);
        }
    }

    TEST(DataTables, _3rd_rank)
    {
        const auto& tables = Chess::DataTables::get();

        Chess::uint64 rank_mask = 0xff;

        EXPECT_EQ(rank_mask << 16,
                  tables._3rd_rank[Chess::player_t::white]);

        EXPECT_EQ(rank_mask << 40,
                  tables._3rd_rank[Chess::player_t::black]);
    }

    TEST(DataTables, a1h8_64)
    {
        const auto& tables = Chess::DataTables::get();

        Chess::uint64 one = 1;

        for (int i = 0; i < 64; i++)
        {
            auto mask = one << i;

            for (int j = i; j < 64; j += 7)
            {
                mask |= one << j;
                if (j % 8 == 0) break;
            }

            for (int j = i; j >= 0; j -= 7)
            {
                mask |= one << j;
                if ((j-7) % 8 == 0) break;
            }

            EXPECT_EQ(tables.a1h8_64[i],
                mask);
        }
    }

    TEST(DataTables, back_rank)
    {
        const auto& tables = Chess::DataTables::get();

        Chess::uint64 ff = 0xff;

        EXPECT_EQ(tables.back_rank[Chess::player_t::white], ff);

        ff <<= 56;

        EXPECT_EQ(tables.back_rank[Chess::player_t::black], ff);
    }

    TEST(DataTables, bishop_attacks)
    {
        const auto& tables = Chess::DataTables::get();

        Chess::uint32 offset = 0;
        for (int sq = 0; sq < 64; sq++)
        {
            std::vector<Chess::uint64> variations;
            get_occupancy_variations(sq, Chess::piece_t::bishop,
                                     variations);

            const int shifts =
                64 - Util::get_msb(variations.size());

            for (auto var : variations)
            {
                Chess::uint64 attacks_from =
                    tables.northeast_mask[sq] |
                    tables.northwest_mask[sq] |
                    tables.southeast_mask[sq] |
                    tables.southwest_mask[sq];

                int blocker =
                    Util::get_lsb(var & tables.northeast_mask[sq]);

                if (blocker != -1)
                    attacks_from ^=
                        tables.northeast_mask[blocker];

                blocker =
                    Util::get_lsb(var & tables.northwest_mask[sq]);

                if (blocker != -1)
                    attacks_from ^=
                        tables.northwest_mask[blocker];

                blocker =
                    Util::get_msb(var & tables.southeast_mask[sq]);

                if (blocker != -1)
                    attacks_from ^=
                        tables.southeast_mask[blocker];

                blocker =
                    Util::get_msb(var & tables.southwest_mask[sq]);

                if (blocker != -1)
                    attacks_from ^=
                        tables.southwest_mask[blocker];

                const Chess::uint32 index = offset +
                    ((var * tables.diag_magics[sq]) >> shifts);

                ASSERT_EQ(tables.bishop_attacks[index],
                    attacks_from);
            }

            offset += variations.size();
        }
    }

    TEST(DataTables, bishop_attacks_mask)
    {
        const auto& tables = Chess::DataTables::get();

        const Chess::uint64 frame =
            Chess::rank_1 | Chess::rank_8 |
            Chess::file_a | Chess::file_h;

        for (int sq = 0; sq < 64; sq++)
        {
            Chess::uint64 mask =
                tables.northwest_mask[sq] |
                tables.northeast_mask[sq] |
                tables.southwest_mask[sq] |
                tables.southeast_mask[sq];

            mask ^= (mask & frame);

            ASSERT_EQ(tables.bishop_attacks_mask[sq],
                mask);
        }
    }

    TEST(DataTables, bishop_db_shifts)
    {
        const auto& tables = Chess::DataTables::get();

        for (int sq = 0; sq < 64; sq++)
        {
            std::vector<Chess::uint64> variations;
            get_occupancy_variations(sq, Chess::piece_t::bishop,
                                     variations);

            const int shift =
                64 - Util::get_msb(variations.size());

            ASSERT_EQ(tables.bishop_db_shifts[sq],
                shift);
        }
    }

    TEST(DataTables, bishop_mobility)
    {
        const auto& tables = Chess::DataTables::get();

        const int numel1 = sizeof(tables.bishop_attacks ) /
            sizeof(tables.bishop_attacks[0]);

        const int numel2 = sizeof(tables.bishop_mobility) /
            sizeof(tables.bishop_mobility[0]);

        static_assert(numel1 == numel2, "");

        for (int i = 0; i < numel1; i++)
        {
            ASSERT_EQ(Util::bit_count(tables.bishop_attacks[i]),
                tables.bishop_mobility[i]);
        }
    }

    TEST(DataTables, bishop_offsets)
    {
        const auto& tables = Chess::DataTables::get();

        int offset = 0;
        for (int sq = 0; sq < 64; sq++)
        {
            ASSERT_EQ(tables.bishop_offsets[sq],
                offset);

            std::vector<Chess::uint64> variations;
            get_occupancy_variations(sq, Chess::piece_t::bishop,
                                     variations);

            offset += variations.size();
        }
    }

    TEST(DataTables, bishop_range_mask)
    {
        const auto& tables = Chess::DataTables::get();

        const Chess::uint64 one = 1;

        for (int sq = 0; sq < 64; sq++)
        {
            Chess::uint64 mask =
                tables.northwest_mask[sq] |
                tables.northeast_mask[sq] |
                tables.southwest_mask[sq] |
                tables.southeast_mask[sq];

            mask |= one << sq;

            ASSERT_EQ(tables.bishop_range_mask[sq],
                mask);
        }
    }

    TEST(DataTables, castle_OO_dest)
    {
        const auto& tables = Chess::DataTables::get();

        EXPECT_EQ(tables.castle_OO_dest[Chess::player_t::white],
                  Chess::square_t::G1);
        EXPECT_EQ(tables.castle_OO_dest[Chess::player_t::black],
                  Chess::square_t::G8);
    }

    TEST(DataTables, castle_OO_path)
    {
        const auto& tables = Chess::DataTables::get();

        Chess::square_t sq1 =
            tables.castle_OO_path[Chess::player_t::white][0];
        Chess::square_t sq2 =
            tables.castle_OO_path[Chess::player_t::white][1];

        EXPECT_NE(sq1, sq2);

        EXPECT_TRUE(sq1 == Chess::square_t::F1 ||
                    sq1 == Chess::square_t::G1);

        EXPECT_TRUE(sq2 == Chess::square_t::F1 ||
                    sq2 == Chess::square_t::G1);

        sq1 =
            tables.castle_OO_path[Chess::player_t::black][0];
        sq2 =
            tables.castle_OO_path[Chess::player_t::black][1];

        EXPECT_NE(sq1, sq2);

        EXPECT_TRUE(sq1 == Chess::square_t::F8 ||
                    sq1 == Chess::square_t::G8);

        EXPECT_TRUE(sq2 == Chess::square_t::F8 ||
                    sq2 == Chess::square_t::G8);
    }

    TEST(DataTables, castle_OOO_dest)
    {
        const auto& tables = Chess::DataTables::get();

        EXPECT_EQ(tables.castle_OOO_dest[Chess::player_t::white],
                  Chess::square_t::C1);
        EXPECT_EQ(tables.castle_OOO_dest[Chess::player_t::black],
                  Chess::square_t::C8);
    }

    TEST(DataTables, castle_OOO_path)
    {
        const auto& tables = Chess::DataTables::get();

        Chess::square_t sq1 =
            tables.castle_OOO_path[Chess::player_t::white][0];
        Chess::square_t sq2 =
            tables.castle_OOO_path[Chess::player_t::white][1];

        EXPECT_NE(sq1, sq2);

        EXPECT_TRUE(sq1 == Chess::square_t::D1 ||
                    sq1 == Chess::square_t::C1);

        EXPECT_TRUE(sq2 == Chess::square_t::D1 ||
                    sq2 == Chess::square_t::C1);

        sq1 =
            tables.castle_OOO_path[Chess::player_t::black][0];
        sq2 =
            tables.castle_OOO_path[Chess::player_t::black][1];

        EXPECT_NE(sq1, sq2);

        EXPECT_TRUE(sq1 == Chess::square_t::D8 ||
                    sq1 == Chess::square_t::C8);

        EXPECT_TRUE(sq2 == Chess::square_t::D8 ||
                    sq2 == Chess::square_t::C8);
    }

    TEST(DataTables, clear_mask)
    {
        const auto& tables = Chess::DataTables::get();

        const Chess::uint64 one = 1;

        for (int sq = 0; sq < 64; sq++)
        {
            ASSERT_EQ(tables.clear_mask[sq],
                      ~(one << sq));
        }
    }

    TEST(DataTables, directions)
    {
        const auto& tables = Chess::DataTables::get();

        for (int i = 0; i < 64; i++)
        {
            for (int j = 0; j < 64; j++)
            {
                if (i == j) continue;

                if (     Chess::get_rank(i) == Chess::get_rank(j))
                {
                    ASSERT_EQ(tables.directions[i][j],
                        Chess::direction_t::along_rank);
                }
                else if (Chess::get_file(i) == Chess::get_file(j))
                {
                    ASSERT_EQ(tables.directions[i][j],
                        Chess::direction_t::along_file);
                }
                else if (tables.h1a8_64[i] & tables.set_mask[j])
                {
                    ASSERT_EQ(tables.directions[i][j],
                        Chess::direction_t::along_h1a8);
                }
                else if (tables.a1h8_64[i] & tables.set_mask[j])
                {
                    ASSERT_EQ(tables.directions[i][j],
                        Chess::direction_t::along_a1h8);
                }
                else
                {
                    ASSERT_EQ(tables.directions[i][j],
                        Chess::direction_t::none);
                }
            }
        }
    }

    TEST(DataTables, east_mask)
    {
        const auto& tables = Chess::DataTables::get();

        const Chess::uint64 one  = 1;
        const Chess::uint64 rank = 0xff;

        for (int i = 0; i < 64; i++)
        {
            EXPECT_EQ(tables.east_mask[i],
                ((one << i)-1) & (rank << (8*(i/8))));
        }
    }

    TEST(DataTables, ep_target)
    {
        const auto& tables = Chess::DataTables::get();

        for (int i = 24; i <= 31; i++)
        {
            EXPECT_EQ(tables.ep_target[i], i-8);
        }

        for (int i = 32; i <= 39; i++)
        {
            EXPECT_EQ(tables.ep_target[i], i+8);
        }
    }

    TEST(DataTables, exchange_array)
    {
        const auto& tables = Chess::DataTables::get();

/*
 * Helper macro to simplify writing this test
 */
#define check_exchange(piece1, piece2)                                         \
{                                                                              \
    EXPECT_EQ(tables.exchange[Chess::piece_t::piece1][Chess::piece_t::piece2], \
              Chess::piece1##_value - Chess::piece2##_value);                  \
}

        check_exchange(pawn, pawn);
        check_exchange(pawn, knight);
        check_exchange(pawn, bishop);
        check_exchange(pawn, rook);
        check_exchange(pawn, queen);
        check_exchange(pawn, king);
        check_exchange(pawn, empty);

        check_exchange(knight, pawn);
        check_exchange(knight, knight);
        check_exchange(knight, bishop);
        check_exchange(knight, rook);
        check_exchange(knight, queen);
        check_exchange(knight, king);
        check_exchange(knight, empty);

        check_exchange(bishop, pawn);
        check_exchange(bishop, knight);
        check_exchange(bishop, bishop);
        check_exchange(bishop, rook);
        check_exchange(bishop, queen);
        check_exchange(bishop, king);
        check_exchange(bishop, empty);

        check_exchange(rook, pawn);
        check_exchange(rook, knight);
        check_exchange(rook, bishop);
        check_exchange(rook, rook);
        check_exchange(rook, queen);
        check_exchange(rook, king);
        check_exchange(rook, empty);

        check_exchange(queen, pawn);
        check_exchange(queen, knight);
        check_exchange(queen, bishop);
        check_exchange(queen, rook);
        check_exchange(queen, queen);
        check_exchange(queen, king);
        check_exchange(queen, empty);

        check_exchange(king, pawn);
        check_exchange(king, knight);
        check_exchange(king, bishop);
        check_exchange(king, rook);
        check_exchange(king, queen);
        check_exchange(king, king);
        check_exchange(king, empty);

        check_exchange(empty, pawn);
        check_exchange(empty, knight);
        check_exchange(empty, bishop);
        check_exchange(empty, rook);
        check_exchange(empty, queen);
        check_exchange(empty, king);
        check_exchange(empty, empty);
    }

    TEST(DataTables, files64)
    {
        const auto& tables = Chess::DataTables::get();

        const Chess::uint64 one    = 1;
        const Chess::uint64 h_file = (one << 0) |
                                     (one << 8) |
                                     (one << 16) |
                                     (one << 24) |
                                     (one << 32) |
                                     (one << 40) |
                                     (one << 48) |
                                     (one << 56);

        for (int i = 0; i < 64; i++)
        {
            EXPECT_EQ(tables.files64[i], h_file << (i%8));
        }
    }

    TEST(DataTables, h1a8_64_array)
    {
        const auto& tables = Chess::DataTables::get();

        Chess::uint64 one = 1;

        for (int i = 0; i < 64; i++)
        {
            auto mask = one << i;

            for (int j = i; j < 64; j += 9)
            {
                mask |= one << j;
                if ((j-7) % 8 == 0) break;
            }

            for (int j = i; j >= 0; j -= 9)
            {
                mask |= one << j;
                if (j % 8 == 0) break;
            }

            EXPECT_EQ(tables.h1a8_64[i],
                mask);
        }
    }

    TEST(DataTables, king_attacks)
    {
        const auto& tables = Chess::DataTables::get();

        const Chess::uint64 one = 1;

        for (int i = 0; i < 64; i++)
        {
            Chess::uint64 mask = 0;

            if (i < 56)
            {
                mask |= one << (i+8);
            }

            if (i < 56 && (i == 0 || (i % 8 != 7)))
            {
                mask |= one << (i+9);
            }

            if (i < 56 && (i % 8 != 0))
            {
                mask |= one << (i+7);
            }

            if (i == 0 || (i % 8 != 7))
            {
                mask |= one << (i+1);
            }

            if (i % 8 != 0)
            {
                mask |= one << (i-1);
            }

            if (i > 7)
            {
                mask |= one << (i-8);
            }

            if (i > 7 && (i % 8 != 7))
            {
                mask |= one << (i-7);
            }

            if (i > 7 && (i % 8 != 0))
            {
                mask |= one << (i-9);
            }

            EXPECT_EQ(tables.king_attacks[i],
                mask) << "square " << i;
        }
    }

    TEST(DataTables, king_home)
    {
        const auto& tables = Chess::DataTables::get();

        EXPECT_EQ(tables.king_home[Chess::player_t::black],
            Chess::square_t::E8);

        EXPECT_EQ(tables.king_home[Chess::player_t::white],
            Chess::square_t::E1);
    }

    TEST(DataTables, kingside)
    {
        const auto& tables = Chess::DataTables::get();

        const Chess::uint64 one = 1;

        EXPECT_EQ( tables.kingside[Chess::player_t::black],
            (one << 58) | (one << 57));

        EXPECT_EQ( tables.kingside[Chess::player_t::white],
            (one << 1 ) | (one << 2 ));
    }

    TEST(DataTables, knight_attacks)
    {
        const auto& tables = Chess::DataTables::get();

        const Chess::uint64 one = 1;

        std::map<Chess::square_t, Chess::uint64>
            attacks_from;

        attacks_from[Chess::square_t::H1] = (one << Chess::square_t::G3) |
                                            (one << Chess::square_t::F2);
        attacks_from[Chess::square_t::G1] = (one << Chess::square_t::F3) |
                                            (one << Chess::square_t::H3) |
                                            (one << Chess::square_t::E2);
        attacks_from[Chess::square_t::B1] = (one << Chess::square_t::A3) |
                                            (one << Chess::square_t::C3) |
                                            (one << Chess::square_t::D2);
        attacks_from[Chess::square_t::A1] = (one << Chess::square_t::B3) |
                                            (one << Chess::square_t::C2);
        attacks_from[Chess::square_t::H2] = (one << Chess::square_t::F1) |
                                            (one << Chess::square_t::F3) |
                                            (one << Chess::square_t::G4);
        attacks_from[Chess::square_t::G2] = (one << Chess::square_t::E1) |
                                            (one << Chess::square_t::E3) |
                                            (one << Chess::square_t::F4) |
                                            (one << Chess::square_t::H4);
        attacks_from[Chess::square_t::B2] = (one << Chess::square_t::A4) |
                                            (one << Chess::square_t::C4) |
                                            (one << Chess::square_t::D3) |
                                            (one << Chess::square_t::D1);
        attacks_from[Chess::square_t::A2] = (one << Chess::square_t::B4) |
                                            (one << Chess::square_t::C3) |
                                            (one << Chess::square_t::C1);
        attacks_from[Chess::square_t::H8] = (one << Chess::square_t::F7) |
                                            (one << Chess::square_t::G6);
        attacks_from[Chess::square_t::G8] = (one << Chess::square_t::E7) |
                                            (one << Chess::square_t::F6) |
                                            (one << Chess::square_t::H6);
        attacks_from[Chess::square_t::B8] = (one << Chess::square_t::D7) |
                                            (one << Chess::square_t::C6) |
                                            (one << Chess::square_t::A6);
        attacks_from[Chess::square_t::A8] = (one << Chess::square_t::C7) |
                                            (one << Chess::square_t::B6);
        attacks_from[Chess::square_t::H7] = (one << Chess::square_t::F8) |
                                            (one << Chess::square_t::F6) |
                                            (one << Chess::square_t::G5);
        attacks_from[Chess::square_t::G7] = (one << Chess::square_t::E8) |
                                            (one << Chess::square_t::E6) |
                                            (one << Chess::square_t::F5) |
                                            (one << Chess::square_t::H5);
        attacks_from[Chess::square_t::B7] = (one << Chess::square_t::A5) |
                                            (one << Chess::square_t::C5) |
                                            (one << Chess::square_t::D6) |
                                            (one << Chess::square_t::D8);
        attacks_from[Chess::square_t::A7] = (one << Chess::square_t::B5) |
                                            (one << Chess::square_t::C6) |
                                            (one << Chess::square_t::C8);

        for (int i = 0; i < 64; i++)
        {
            auto iter = attacks_from.find(
                static_cast<Chess::square_t>(i));

            if (iter == attacks_from.end())
            {
                Chess::uint64 expected = 0;

                switch (Chess::get_rank(i))
                {
                case 0:
                    expected = (one << (i+ 6)) |
                               (one << (i+10)) |
                               (one << (i+15)) | 
                               (one << (i+17));
                    break;
                case 1:
                    expected = (one << (i-10)) |
                               (one << (i- 6)) |
                               (one << (i+ 6)) |
                               (one << (i+10)) |
                               (one << (i+15)) | 
                               (one << (i+17));
                    break;
                case 7:
                    expected = (one << (i- 6)) |
                               (one << (i-10)) |
                               (one << (i-15)) | 
                               (one << (i-17));
                    break;
                case 6:
                    expected = (one << (i-10)) |
                               (one << (i- 6)) |
                               (one << (i+ 6)) |
                               (one << (i+10)) |
                               (one << (i-15)) | 
                               (one << (i-17));
                default:
                    break;
                }

                switch (Chess::get_file(i))
                {
                case 0:
                    expected = (one << (i- 6)) |
                               (one << (i+10)) |
                               (one << (i-15)) | 
                               (one << (i+17));
                    break;
                case 1:
                    expected = (one << (i- 6)) |
                               (one << (i+10)) |
                               (one << (i-15)) | 
                               (one << (i-17)) |
                               (one << (i+15)) | 
                               (one << (i+17));
                    break;
                case 7:
                    expected = (one << (i+ 6)) |
                               (one << (i-10)) |
                               (one << (i+15)) | 
                               (one << (i-17));
                    break;
                case 6:
                    expected = (one << (i+ 6)) |
                               (one << (i-10)) |
                               (one << (i+15)) | 
                               (one << (i-17)) |
                               (one << (i-15)) | 
                               (one << (i+17));
                default:
                    break;
                }

                if (expected == 0)
                {
                    // Knight is somewhere in the center

                    expected = (one << (i+ 6)) |
                               (one << (i+10)) |
                               (one << (i- 6)) |
                               (one << (i-10)) |
                               (one << (i+15)) | 
                               (one << (i+17)) |
                               (one << (i-15)) | 
                               (one << (i-17));
                }

                EXPECT_EQ(expected, tables.knight_attacks[i])
                    << "Failed on square " << i;
            }
            else
            {
                Chess::uint64 expected = iter->second;
                EXPECT_EQ(expected, tables.knight_attacks[i])
                    << "Failed on square " << i;
            }
        }
    }

    TEST(DataTables, lsb)
    {
        const auto& tables = Chess::DataTables::get();

        auto lsb = [](Chess::uint16 word) {
            if (word == 0) return((Chess::uint8)0xff);

            Chess::uint8 lsb = 0;
            
            while (((1 << lsb) & word) == 0)
                lsb++;

            return lsb;
        };

        for (Chess::uint32 i = 1; i <= 65535; i++)
        {
            ASSERT_EQ(tables.lsb[i], lsb(i));
        }
    }

    TEST(DataTables, minus_16)
    {
        const auto& tables = Chess::DataTables::get();

        for (int i = 0; i < 64; i++)
        {
            const int actual_w =
                tables.minus_16[Chess::player_t::white][i];
            const int actual_b =
                tables.minus_16[Chess::player_t::black][i];

            if (i > 15)
            {
                EXPECT_EQ( actual_w, i-16 );
            }
            else
            {
                EXPECT_EQ(Chess::square_t::BAD_SQUARE,
                    actual_w);
            }

            if (i < 48)
            {
                EXPECT_EQ( actual_b, i+16 );
            }
            else
            {
                EXPECT_EQ(Chess::square_t::BAD_SQUARE,
                    actual_b);
            }
        }
    }

    TEST(DataTables, minus_8)
    {
        const auto& tables = Chess::DataTables::get();

        for (int i = 0; i < 64; i++)
        {
            const int actual_w =
                tables.minus_8[Chess::player_t::white][i];
            const int actual_b =
                tables.minus_8[Chess::player_t::black][i];

            if (i > 7)
            {
                EXPECT_EQ( actual_w, i-8 );
            }
            else
            {
                EXPECT_EQ(Chess::square_t::BAD_SQUARE,
                    actual_w);
            }

            if (i < 56)
            {
                EXPECT_EQ( actual_b, i+8 );
            }
            else
            {
                EXPECT_EQ(Chess::square_t::BAD_SQUARE,
                    actual_b);
            }
        }
    }

    TEST(DataTables, minus_9)
    {
        const auto& tables = Chess::DataTables::get();

        for (int i = 0; i < 64; i++)
        {
            const Chess::square_t actual =
                tables.minus_9[ Chess::player_t::white ][i];

            if (Chess::get_file(i) == 0 || i-9 < 0)
            {
                EXPECT_EQ(Chess::square_t::BAD_SQUARE,
                          actual);
            }
            else
            {
                EXPECT_EQ(static_cast<Chess::square_t>(i-9),
                          actual);
            }
        }

        for (int i = 0; i < 64; i++)
        {
            const Chess::square_t actual =
                tables.minus_9[ Chess::player_t::black ][i];

            if (Chess::get_file(i) == 7 || i+9 > 63)
            {
                EXPECT_EQ(Chess::square_t::BAD_SQUARE,
                          actual);
            }
            else
            {
                EXPECT_EQ(static_cast<Chess::square_t>(i+9),
                          actual);
            }
        }
    }

    TEST(DataTables, minus_7)
    {
        const auto& tables = Chess::DataTables::get();

        for (int i = 0; i < 64; i++)
        {
            const Chess::square_t actual =
                tables.minus_7[ Chess::player_t::white ][i];

            if (Chess::get_file(i) == 7 || i-7 < 0)
            {
                EXPECT_EQ(Chess::square_t::BAD_SQUARE,
                          actual);
            }
            else
            {
                EXPECT_EQ(static_cast<Chess::square_t>(i-7),
                          actual);
            }
        }

        for (int i = 0; i < 64; i++)
        {
            const Chess::square_t actual =
                tables.minus_7[ Chess::player_t::black ][i];

            if (Chess::get_file(i) == 0 || i+7 > 63)
            {
                EXPECT_EQ(Chess::square_t::BAD_SQUARE,
                          actual);
            }
            else
            {
                EXPECT_EQ(static_cast<Chess::square_t>(i+7),
                          actual);
            }
        }
    }

    TEST(DataTables, msb)
    {
        const auto& tables = Chess::DataTables::get();

        auto msb = [](Chess::uint16 word) {
            if (word == 0) return((Chess::uint8)0xff);

            Chess::uint8 msb = 15;
            
            while (((1 << msb) & word) == 0)
                msb--;

            return msb;
        };

        for (Chess::uint32 i = 1; i <= 65535; i++)
        {
            ASSERT_EQ(tables.msb[i], msb(i));
        }
    }

    TEST(DataTables, north_mask)
    {
        const auto& tables = Chess::DataTables::get();

        auto create_mask = [](int i) {
            Chess::uint64 mask = 0, one = 1;

            for(int j = i+8; j < 64; j += 8)
                mask |= one << j;
            
            return mask;
        };

        for (int i = 0; i < 64; i++)
        {
            ASSERT_EQ(tables.north_mask[i],
                create_mask(i));
        }
    }

    TEST(DataTables, northeast_mask)
    {
        const auto& tables = Chess::DataTables::get();

        auto create_mask = [](int i) {
            Chess::uint64 mask = 0, one = 1;

            while (Chess::get_file(i) > 0)
            {
                i += 7;
                if (i < 64) mask |= one << i;
            }
            
            return mask;
        };

        for (int i = 0; i < 64; i++)
        {
            ASSERT_EQ(tables.northeast_mask[i],
                create_mask(i)) << i;
        }
    }

    TEST(DataTables, northwest_mask)
    {
        const auto& tables = Chess::DataTables::get();

        auto create_mask = [](int i) {
            Chess::uint64 mask = 0, one = 1;

            while (Chess::get_file(i) < 7)
            {
                i += 9;
                if (i < 64) mask |= one << i;
            }
            
            return mask;
        };

        for (int i = 0; i < 64; i++)
        {
            ASSERT_EQ(tables.northwest_mask[i],
                create_mask(i)) << i;
        }
    }

    TEST(DataTables, pawn_advances)
    {
        const auto& tables = Chess::DataTables::get();

        auto create_mask =
            [](int i, Chess::player_t who) {

            Chess::uint64 mask = 0, one = 1;

            if (Chess::get_rank(i) == 0 || Chess::get_rank(i) == 7)
                return mask;

            if (who == Chess::player_t::white)
            {
                if (Chess::get_rank(i) <= 6)
                {
                    mask |= one << (i+8);
                }

                if (Chess::get_rank(i) == 1)
                {
                    mask |= one << (i+16);
                }
            }
            else
            {
                if (Chess::get_rank(i) >= 1)
                {
                    mask |= one << (i-8);
                }

                if (Chess::get_rank(i) == 6)
                {
                    mask |= one << (i-16);
                }
            }
            
            return mask;
        };

        for (int i = 8; i < 56; i++)
        {
            ASSERT_EQ(tables.pawn_advances[Chess::player_t::white][i],
                create_mask(i, Chess::player_t::white)) << i;
            ASSERT_EQ(tables.pawn_advances[Chess::player_t::black][i],
                create_mask(i, Chess::player_t::black)) << i;
        }
    }

    TEST(DataTables, pawn_attacks)
    {
        const auto& tables = Chess::DataTables::get();

        auto create_mask =
            [](int i, Chess::player_t who) {

            Chess::uint64 mask = 0, one = 1;

            if (Chess::get_rank(i) == 0 || Chess::get_rank(i) == 7)
                return mask;

            if (who == Chess::player_t::white)
            {
                if (Chess::get_file(i) < 7 && i < 56)
                {
                    mask |= one << (i+9);
                }

                if (Chess::get_file(i) > 0 && i < 56)
                {
                    mask |= one << (i+7);
                }
            }
            else
            {
                if (Chess::get_file(i) < 7 && i >= 0)
                {
                    mask |= one << (i-7);
                }

                if (Chess::get_file(i) > 0 && i >= 0)
                {
                    mask |= one << (i-9);
                }
            }
            
            return mask;
        };

        for (int i = 8; i < 56; i++)
        {
            ASSERT_EQ(tables.pawn_attacks[Chess::player_t::white][i],
                create_mask(i, Chess::player_t::white)) << i;
            ASSERT_EQ(tables.pawn_attacks[Chess::player_t::black][i],
                create_mask(i, Chess::player_t::black)) << i;
        }
    }

    TEST(DataTables, piece_value)
    {
        const auto& tables = Chess::DataTables::get();

        EXPECT_EQ(tables.piece_value[Chess::piece_t::knight],
                  Chess::knight_value);
        EXPECT_EQ(tables.piece_value[Chess::piece_t::bishop],
                  Chess::bishop_value);
        EXPECT_EQ(tables.piece_value[Chess::piece_t::pawn],
                  Chess::pawn_value);
        EXPECT_EQ(tables.piece_value[Chess::piece_t::rook],
                  Chess::rook_value);
        EXPECT_EQ(tables.piece_value[Chess::piece_t::queen],
                  Chess::queen_value);
        EXPECT_EQ(tables.piece_value[Chess::piece_t::king],
                  Chess::king_value);
    }

    TEST(DataTables, plus_8)
    {
        const auto& tables = Chess::DataTables::get();

        for (int i = 0; i < 64; i++)
        {
            const int actual_w =
                tables.plus_8[Chess::player_t::white][i];
            const int actual_b =
                tables.plus_8[Chess::player_t::black][i];

            if (i < 56)
            {
                EXPECT_EQ( actual_w, i+8 );
            }
            else
            {
                EXPECT_EQ(Chess::square_t::BAD_SQUARE,
                    actual_w);
            }

            if (i > 7 )
            {
                EXPECT_EQ( actual_b, i-8 );
            }
            else
            {
                EXPECT_EQ(Chess::square_t::BAD_SQUARE,
                    actual_b);
            }
        }
    }

    TEST(DataTables, plus_9)
    {
        const auto& tables = Chess::DataTables::get();

        for (int i = 0; i < 64; i++)
        {
            const Chess::square_t actual =
                tables.plus_9[ Chess::player_t::white ][i];

            if (Chess::get_file(i) == 7 || i+9 > 63)
            {
                EXPECT_EQ(Chess::square_t::BAD_SQUARE,
                          actual);
            }
            else
            {
                EXPECT_EQ(static_cast<Chess::square_t>(i+9),
                          actual);
            }
        }

        for (int i = 0; i < 64; i++)
        {
            const Chess::square_t actual =
                tables.plus_9[ Chess::player_t::black ][i];

            if (Chess::get_file(i) == 0 || i-9 < 0)
            {
                EXPECT_EQ(Chess::square_t::BAD_SQUARE,
                          actual);
            }
            else
            {
                EXPECT_EQ(static_cast<Chess::square_t>(i-9),
                          actual);
            }
        }
    }

    TEST(DataTables, plus_7)
    {
        const auto& tables = Chess::DataTables::get();

        for (int i = 0; i < 64; i++)
        {
            const Chess::square_t actual =
                tables.plus_7[ Chess::player_t::white ][i];

            if (Chess::get_file(i) == 0 || i+7 > 63)
            {
                EXPECT_EQ(Chess::square_t::BAD_SQUARE,
                          actual);
            }
            else
            {
                EXPECT_EQ(static_cast<Chess::square_t>(i+7),
                          actual);
            }
        }

        for (int i = 0; i < 64; i++)
        {
            const Chess::square_t actual =
                tables.plus_7[ Chess::player_t::black ][i];

            if (Chess::get_file(i) == 7 || i-7 < 0)
            {
                EXPECT_EQ(Chess::square_t::BAD_SQUARE,
                          actual);
            }
            else
            {
                EXPECT_EQ(static_cast<Chess::square_t>(i-7),
                          actual);
            }
        }
    }

    TEST(DataTables, pop)
    {
        const auto& tables = Chess::DataTables::get();

        auto pop = [](Chess::uint16 word) {

            Chess::uint8 pop = 0;

            for (; word; word >>= 1)
                pop += word & 1;

            return pop;
        };

        for (Chess::uint32 i = 0; i <= 65535; i++)
        {
            ASSERT_EQ(tables.pop[i], pop(i));
        }
    }

    TEST(DataTables, rank_adjacent)
    {
        const auto& tables = Chess::DataTables::get();

        const Chess::uint64 one = 1;

        for ( int i = 0; i <= 63; i++ )
        {
            Chess::uint64 expected = 0;

            if (Chess::get_rank(i) == Chess::get_rank(i+1))
            {
                expected |= one << (i+1);
            }

            if (Chess::get_rank(i) == Chess::get_rank(i-1))
            {
                expected |= one << (i-1);
            }

            ASSERT_EQ(tables.rank_adjacent[i],
                expected);
        }
    }

    TEST(DataTables, ranks64)
    {
        const auto& tables = Chess::DataTables::get();

        const Chess::uint64 one = 1;

        for ( int i = 0; i <= 63; i++ )
        {
            Chess::uint64 expected = 0;

            for (int j = i; Chess::get_rank(j) == Chess::get_rank(i);
                 j++)
            {
                expected |= one << j;
            }

            for (int j = i; Chess::get_rank(j) == Chess::get_rank(i);
                 j--)
            {
                expected |= one << j;
            }

            ASSERT_EQ(tables.ranks64[i],
                expected);
        }
    }

    TEST(DataTables, ray)
    {
        const auto& tables = Chess::DataTables::get();

        const Chess::uint64 one = 1;

        for ( int i = 0; i <= 63; i++ )
        {
            for ( int j = 0; j <= 63; j++ )
            {
                if (i == j)
                {
                    ASSERT_EQ(tables.ray[i][j], 0);
                }
                else
                {
                    if (tables.northeast_mask[i] & tables.set_mask[j])
                    {
                        const Chess::uint64 expected =
                            (one << i) | tables.northeast_mask[i];
                        ASSERT_EQ(expected, tables.ray[i][j]);
                    }
                    else if (tables.northwest_mask[i] & tables.set_mask[j])
                    {
                        const Chess::uint64 expected =
                            (one << i) | tables.northwest_mask[i];
                        ASSERT_EQ(expected, tables.ray[i][j]);
                    }
                    else if (tables.southeast_mask[i] & tables.set_mask[j])
                    {
                        const Chess::uint64 expected =
                            (one << i) | tables.southeast_mask[i];
                        ASSERT_EQ(expected, tables.ray[i][j]);
                    }
                    else if (tables.southwest_mask[i] & tables.set_mask[j])
                    {
                        const Chess::uint64 expected =
                            (one << i) | tables.southwest_mask[i];
                        ASSERT_EQ(expected, tables.ray[i][j]);
                    }
                    else if (tables.east_mask[i] & tables.set_mask[j])
                    {
                        const Chess::uint64 expected =
                                (one << i) | tables.east_mask[i];
                        ASSERT_EQ(expected, tables.ray[i][j]);
                    }
                    else if (tables.west_mask[i] & tables.set_mask[j])
                    {
                        const Chess::uint64 expected =
                                (one << i) | tables.west_mask[i];
                        ASSERT_EQ(expected, tables.ray[i][j]);
                    }
                    else if (tables.north_mask[i] & tables.set_mask[j])
                    {
                        const Chess::uint64 expected =
                                (one << i) | tables.north_mask[i];
                        ASSERT_EQ(expected, tables.ray[i][j]);
                    }
                    else if (tables.south_mask[i] & tables.set_mask[j])
                    {
                        const Chess::uint64 expected =
                                (one << i) | tables.south_mask[i];
                        ASSERT_EQ(expected, tables.ray[i][j]);
                    }
                }
            } // for j ...
        } // for i ...
    }

    TEST(DataTables, ray_extend)
    {
        const auto& tables = Chess::DataTables::get();

        const Chess::uint64 one = 1;

        auto along_a1h8 = []( int sq1, int sq2 ) {
            if (sq1 == sq2) return false;

            int i   = std::min(sq1, sq2);
            int tgt = std::max(sq1, sq2);

            for (; i < 64; i += 7 )
            {
                if (i == tgt) return true;

                if ( Chess::get_file(i) == 0 )
                    break;
            }

            return false;
        };

        auto along_h1a8 = []( int sq1, int sq2 ) {
            if (sq1 == sq2) return false;

            int i   = std::min(sq1, sq2);
            int tgt = std::max(sq1, sq2);

            for (; i < 64; i += 9 )
            {
                if (i == tgt) return true;

                if ( Chess::get_file(i) == 7 )
                    break;
            }

            return false;
        };

        for ( int i = 0; i <= 63; i++ )
        {
            for ( int j = 0; j <= 63; j++ )
            {
                if (i == j)
                {
                    ASSERT_EQ(tables.ray_extend[i][j], 0);
                }
                else
                {
                    Chess::uint64 expected = 0;

                    if   (   Chess::get_file(i) == Chess::get_file(j))
                    {
                        expected =
                            (one << i) | tables.north_mask[i]|
                                         tables.south_mask[i];
                    }
                    else if (Chess::get_rank(i) == Chess::get_rank(j))
                    {
                        expected =
                            (one << i) | tables.east_mask[i]|
                                         tables.west_mask[i];
                    }
                    else if ( along_h1a8(i,j) )
                    {
                        expected =
                            (one << i) | tables.southeast_mask[i]|
                                         tables.northwest_mask[i];
                    }
                    else if ( along_a1h8(i,j) )
                    {
                        expected =
                            (one << i) | tables.northeast_mask[i]|
                                         tables.southwest_mask[i];
                    }

                    ASSERT_EQ(tables.ray_extend[i][j],
                        expected) << i << ", " << j;
                }
            } // for j ...
        } // for i ...
    }

    TEST(DataTables, ray_segment)
    {
        const auto& tables = Chess::DataTables::get();

        auto along_a1h8 = []( int sq1, int sq2 ) {
            if (sq1 == sq2) return false;

            int i   = std::min(sq1, sq2);
            int tgt = std::max(sq1, sq2);

            for (; i < 64; i += 7 )
            {
                if (i == tgt) return true;

                if ( Chess::get_file(i) == 0 )
                    break;
            }

            return false;
        };

        auto along_h1a8 = []( int sq1, int sq2 ) {
            if (sq1 == sq2) return false;

            int i   = std::min(sq1, sq2);
            int tgt = std::max(sq1, sq2);

            for (; i < 64; i += 9 )
            {
                if (i == tgt) return true;

                if ( Chess::get_file(i) == 7 )
                    break;
            }

            return false;
        };

        for ( int i = 0; i <= 63; i++ )
        {
            for ( int j = 0; j <= 63; j++ )
            {
                if (i == j)
                {
                    ASSERT_EQ(tables.ray_segment[i][j], 0);
                }
                else
                {
                    Chess::uint64 expected = 0;

                    if   (   Chess::get_file(i) == Chess::get_file(j))
                    {
                        if (i > j)
                        {
                            expected = tables.north_mask[j] &
                                       tables.south_mask[i];
                        }
                        else
                        {
                            expected = tables.north_mask[i] &
                                       tables.south_mask[j];
                        }
                    }
                    else if (Chess::get_rank(i) == Chess::get_rank(j))
                    {
                        if (i > j)
                        {
                            expected =  tables.west_mask[j] &
                                        tables.east_mask[i];
                        }
                        else
                        {
                            expected =  tables.west_mask[i] &
                                        tables.east_mask[j];
                        }
                    }
                    else if ( along_h1a8(i,j) )
                    {
                        if (i > j)
                        {
                            expected = tables.northwest_mask[j] &
                                       tables.southeast_mask[i];
                        }
                        else
                        {
                            expected = tables.northwest_mask[i] &
                                       tables.southeast_mask[j];
                        }
                    }
                    else if ( along_a1h8(i,j) )
                    {
                        if (i > j)
                        {
                            expected = tables.northeast_mask[j] &
                                       tables.southwest_mask[i];
                        }
                        else
                        {
                            expected = tables.northeast_mask[i] &
                                       tables.southwest_mask[j];
                        }
                    }

                    ASSERT_EQ(tables.ray_segment[i][j],
                        expected) << i << ", " << j;
                }
            } // for j ...
        } // for i ...
    }

    TEST(DataTables, rook_attacks)
    {
        const auto& tables = Chess::DataTables::get();

        Chess::uint32 offset = 0;
        for (int sq = 0; sq < 64; sq++)
        {
            std::vector<Chess::uint64> variations;
            get_occupancy_variations(sq, Chess::piece_t::rook,
                                     variations);

            const int shifts =
                64 - Util::get_msb(variations.size());

            for (auto var : variations)
            {
                Chess::uint64 attacks_from =
                    tables.north_mask[sq] |
                    tables.west_mask[sq]  |
                    tables.south_mask[sq] |
                    tables.east_mask[sq];

                int blocker =
                    Util::get_lsb(var & tables.north_mask[sq]);

                if (blocker != -1)
                    attacks_from ^=
                        tables.north_mask[blocker];

                blocker =
                    Util::get_lsb(var & tables.west_mask[sq]);

                if (blocker != -1)
                    attacks_from ^=
                        tables.west_mask[blocker];

                blocker =
                    Util::get_msb(var & tables.south_mask[sq]);

                if (blocker != -1)
                    attacks_from ^=
                        tables.south_mask[blocker];

                blocker =
                    Util::get_msb(var & tables.east_mask[sq]);

                if (blocker != -1)
                    attacks_from ^=
                        tables.east_mask[blocker];

                const Chess::uint32 index = offset +
                    ((var * tables.rook_magics[sq]) >> shifts);

                ASSERT_EQ(tables.rook_attacks[index],
                    attacks_from);
            }

            offset += variations.size();
        }
    }

    TEST(DataTables, rook_attacks_mask)
    {
        const auto& tables = Chess::DataTables::get();

        const Chess::uint64 frame =
            Chess::rank_1 | Chess::rank_8 |
            Chess::file_a | Chess::file_h;

        const Chess::uint64 one     = 1;
        const Chess::uint64 corners =
            (one << 63) | (one << 56) | (one << 7) |
            (one << 0);

        for (int sq = 0; sq < 64; sq++)
        {
            Chess::uint64 mask =
                tables.north_mask[sq] |
                tables.east_mask[sq]  |
                tables.south_mask[sq] |
                tables.west_mask[sq];

            if (sq == 0 || sq == 7 || sq == 56 || sq == 63)
            {
                mask ^= (mask & corners);
            }
            else if (Chess::get_rank(sq) == 0)
            {
                mask ^= (mask & (Chess::rank_8 | corners));
            }
            else if (Chess::get_rank(sq) == 7)
            {
                mask ^= (mask & (Chess::rank_1 | corners));
            }
            else if (Chess::get_file(sq) == 0)
            {
                mask ^= (mask & (Chess::file_a | corners));
            }
            else if (Chess::get_file(sq) == 7)
            {
                mask ^= (mask & (Chess::file_h | corners));
            }
            else
            {
                mask ^= (mask & frame);
            }

            ASSERT_EQ(tables.rook_attacks_mask[sq],
                mask) << sq;
        }
    }

    TEST(DataTables, rook_db_shifts)
    {
        const auto& tables = Chess::DataTables::get();

        for (int sq = 0; sq < 64; sq++)
        {
            std::vector<Chess::uint64> variations;
            get_occupancy_variations(sq, Chess::piece_t::rook,
                                     variations);

            const int shift =
                64 - Util::get_msb(variations.size());

            ASSERT_EQ(tables.rook_db_shifts[sq],
                shift);
        }
    }

    TEST(DataTables, rook_mobility)
    {
        const auto& tables = Chess::DataTables::get();

        const int numel1 = sizeof(tables.rook_attacks ) /
            sizeof(tables.rook_attacks[0]);

        const int numel2 = sizeof(tables.rook_mobility) /
            sizeof(tables.rook_mobility[0]);

        static_assert(numel1 == numel2, "");

        for (int i = 0; i < numel1; i++)
        {
            ASSERT_EQ(Util::bit_count(tables.rook_attacks[i]),
                tables.rook_mobility[i]);
        }
    }

    TEST(DataTables, rook_offsets)
    {
        const auto& tables = Chess::DataTables::get();

        int offset = 0;
        for (int sq = 0; sq < 64; sq++)
        {
            ASSERT_EQ(tables.rook_offsets[sq],
                offset);

            std::vector<Chess::uint64> variations;
            get_occupancy_variations(sq, Chess::piece_t::rook,
                                     variations);

            offset += variations.size();
        }
    }

    TEST(DataTables, rook_range_mask)
    {
        const auto& tables = Chess::DataTables::get();

        const Chess::uint64 one = 1;

        for (int sq = 0; sq < 64; sq++)
        {
            Chess::uint64 mask =
                tables.north_mask[sq] |
                tables.east_mask[sq]  |
                tables.south_mask[sq] |
                tables.west_mask[sq];

            mask |= one << sq;

            ASSERT_EQ(tables.rook_range_mask[sq],
                mask);
        }
    }

    TEST(DataTables, queenside)
    {
        const auto& tables = Chess::DataTables::get();

        const Chess::uint64 one = 1;

        EXPECT_EQ(tables.queenside[Chess::player_t::white],
            (one << 6 ) | (one << 5 ) | (one << 4 ));
        EXPECT_EQ(tables.queenside[Chess::player_t::black],
            (one << 62) | (one << 61) | (one << 60));
    }

    TEST(DataTables, set_mask)
    {
        const auto& tables = Chess::DataTables::get();

        const Chess::uint64 one = 1;

        for (int i = 0; i < 64; i++)
        {
            EXPECT_EQ( tables.set_mask[i], one << i );
        }
    }

    TEST(DataTables, sign)
    {
        const auto& tables = Chess::DataTables::get();

        EXPECT_EQ(tables.sign[Chess::player_t::white],
            1);
        EXPECT_EQ(tables.sign[Chess::player_t::black],
            -1);
    }

    TEST(DataTables, south_mask)
    {
        const auto& tables = Chess::DataTables::get();

        auto create_mask = [](int i) {
            Chess::uint64 mask = 0, one = 1;

            for(int j = i-8; j >= 0; j -= 8)
                mask |= one << j;
            
            return mask;
        };

        for (int i = 0; i < 64; i++)
        {
            ASSERT_EQ(tables.south_mask[i],
                create_mask(i));
        }
    }

    TEST(DataTables, southeast_mask)
    {
        const auto& tables = Chess::DataTables::get();

        auto create_mask = [](int i) {
            Chess::uint64 mask = 0, one = 1;

            while (Chess::get_file(i) > 0)
            {
                i -= 9;
                if (i >= 0) mask |= one << i;
            }
            
            return mask;
        };

        for (int i = 0; i < 64; i++)
        {
            ASSERT_EQ(tables.southeast_mask[i],
                create_mask(i)) << i;
        }
    }

    TEST(DataTables, southwest_mask)
    {
        const auto& tables = Chess::DataTables::get();

        auto create_mask = [](int i) {
            Chess::uint64 mask = 0, one = 1;

            while (Chess::get_file(i) < 7)
            {
                i -= 7;
                if (i >= 0) mask |= one << i;
            }
            
            return mask;
        };

        for (int i = 0; i < 64; i++)
        {
            ASSERT_EQ(tables.southwest_mask[i],
                create_mask(i)) << i;
        }
    }

    TEST(DataTables, west_mask)
    {
        const auto& tables = Chess::DataTables::get();

        const Chess::uint64 one = 1;

        for (int i = 0; i < 64; i++)
        {
            Chess::uint64 expected = 0;

            if (Chess::get_file(i) < 7)
            {
                for (int j = i+1;
                     Chess::get_rank(j) == Chess::get_rank(i);
                     j++)
                {
                    expected |= (one << j);
                }
            }

            ASSERT_EQ( tables.west_mask[i],
                expected ) << i;
        }
    }
}
