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

    TEST(DataTables, _3rd_rank_array)
    {
        const auto& tables = Chess::DataTables::get();

        Chess::uint64 rank_mask = 0xff;

        EXPECT_EQ(rank_mask << 16,
                  tables._3rd_rank[Chess::player_t::white]);

        EXPECT_EQ(rank_mask << 40,
                  tables._3rd_rank[Chess::player_t::black]);
    }

    TEST(DataTables, a1h8_64_array)
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

    TEST(DataTables, back_rank_array)
    {
        const auto& tables = Chess::DataTables::get();

        Chess::uint64 ff = 0xff;

        EXPECT_EQ(tables.back_rank[Chess::player_t::white], ff);

        ff <<= 56;

        EXPECT_EQ(tables.back_rank[Chess::player_t::black], ff);
    }

    TEST(DataTables, bishop_attacks_array)
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
}
