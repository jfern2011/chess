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

        for (int i = 16; i < 64; i++)
        {
            const int actual =
                tables.minus_16[Chess::player_t::white][i];
            EXPECT_EQ( actual, i-16 );
        }

        for (int i = 0 ; i < 48; i++)
        {
            const int actual =
                tables.minus_16[Chess::player_t::black][i];
            EXPECT_EQ( actual, i+16 );
        }
    }

    TEST(DataTables, minus_8)
    {
        const auto& tables = Chess::DataTables::get();

        for (int i = 8; i < 64; i++)
        {
            const int actual =
                tables.minus_8 [Chess::player_t::white][i];
            EXPECT_EQ( actual, i-8 );
        }

        for (int i = 0 ; i < 56; i++)
        {
            const int actual =
                tables.minus_8 [Chess::player_t::black][i];
            EXPECT_EQ( actual, i+8 );
        }
    }

    TEST(DataTables, minus_9)
    {
        const auto& tables = Chess::DataTables::get();

        for (int i = 9; i < 64; i++)
        {
            if (Chess::get_file(i) == 0)
                continue;

            const int actual =
                tables.minus_9 [Chess::player_t::white][i];
            EXPECT_EQ( actual, i-9 );
        }

        for (int i = 0; i < 55; i++)
        {
            if (Chess::get_file(i) == 7)
                continue;

            const int actual =
                tables.minus_9 [Chess::player_t::black][i];
            EXPECT_EQ( actual, i+9 );
        }
    }

    TEST(DataTables, minus_7)
    {
        const auto& tables = Chess::DataTables::get();

        for (int i = 7; i < 64; i++)
        {
            if (Chess::get_file(i) == 7)
                continue;

            const int actual =
                tables.minus_7 [Chess::player_t::white][i];
            EXPECT_EQ( actual, i-7 );
        }

        for (int i = 0; i < 56; i++)
        {
            if (Chess::get_file(i) == 0)
                continue;

            const int actual =
                tables.minus_7 [Chess::player_t::black][i];
            EXPECT_EQ( actual, i+7 );
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
}
