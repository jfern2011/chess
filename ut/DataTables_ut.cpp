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
}
