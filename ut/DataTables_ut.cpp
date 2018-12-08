#include "gtest/gtest.h"

/*
 * Note: Include *after* gtest to protect against
 *       name conflicts with abort macros
 */
#include "src/DataTables4.h"

namespace
{
	TEST(DataTables, a1h8_64_array)
	{
		auto& tables = Chess::DataTables::get();

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
		auto& tables = Chess::DataTables::get();

		Chess::uint64 ff = 0xff;

		EXPECT_EQ(tables.back_rank[Chess::player_t::white], ff);

		ff <<= 56;

		EXPECT_EQ(tables.back_rank[Chess::player_t::black], ff);
	}

	TEST(DataTables, bishop_attacks_array)
	{

	}

	TEST(DataTables, h1a8_64_array)
	{
		auto& tables = Chess::DataTables::get();

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
		auto& tables = Chess::DataTables::get();

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
