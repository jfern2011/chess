#include <functional>

#include "gtest/gtest.h"
#include "src/SelectionSort.h"
#include "src/chess_util4.h"

using namespace Chess;

namespace
{
	auto cmp_fun = []( int32 mv1, int32 mv2 ) {
		return score(mv1) - score(mv2);
	};

	TEST(selection_sort, one_item)
	{
		int32 move = pack_move(piece_t::rook,
							   square_t::E7,
							   piece_t::pawn,
							   piece_t::empty,
							   square_t::F8);

		SelectionSort sort;

		sort.init(&move,1);

		int32 next;
		EXPECT_TRUE (sort.next(next, cmp_fun));
		EXPECT_EQ(next, move);

		next = 0;
		EXPECT_FALSE(sort.next(next, cmp_fun));
		EXPECT_EQ(next, 0);

		next = 0;
		EXPECT_FALSE(sort.next(next, cmp_fun));
		EXPECT_EQ(next, 0);
	}

	TEST(selection_sort, two_items)
	{
		int32 moves[2];

		{
			moves[0] = pack_move(piece_t::rook,
								 square_t::E7,
								 piece_t::pawn,
								 piece_t::empty,
								 square_t::F8);

			moves[1] = pack_move(piece_t::rook,
								 square_t::E7,
								 piece_t::knight,
								 piece_t::empty,
								 square_t::G6);

			int32 move0 = moves[0];
			int32 move1 = moves[1];

			SelectionSort sort;

			sort.init(moves,2);

			int32 next;
			EXPECT_TRUE (sort.next(next, cmp_fun));
			EXPECT_EQ(next, move0);

			next = 0;
			EXPECT_TRUE (sort.next(next, cmp_fun));
			EXPECT_EQ(next, move1);

			next = 0;
			EXPECT_FALSE(sort.next(next, cmp_fun));
			EXPECT_EQ(next, 0);
		}

		{
			moves[0] = pack_move(piece_t::rook,
								 square_t::E7,
								 piece_t::knight,
								 piece_t::empty,
								 square_t::G6);

			moves[1] = pack_move(piece_t::rook,
								 square_t::E7,
								 piece_t::pawn,
								 piece_t::empty,
								 square_t::F8);

			int32 move0 = moves[0];
			int32 move1 = moves[1];

			SelectionSort sort;

			sort.init(moves,2);

			int32 next;
			EXPECT_TRUE (sort.next(next, cmp_fun));
			EXPECT_EQ(next, move1);

			next = 0;
			EXPECT_TRUE (sort.next(next, cmp_fun));
			EXPECT_EQ(next, move0);

			next = 0;
			EXPECT_FALSE(sort.next(next, cmp_fun));
			EXPECT_EQ(next, 0);
		}
	}

	TEST(selection_sort, ten_items_reversed)
	{
		int32 moves[10];

		{
			moves[9] = pack_move(piece_t::queen,  // 875
								 square_t::E7,
								 piece_t::pawn,
								 piece_t::empty,
								 square_t::F8);

			moves[8] = pack_move(piece_t::queen,  // 650
								 square_t::E7,
								 piece_t::knight,
								 piece_t::empty,
								 square_t::F8);

			moves[7] = pack_move(piece_t::queen,  // 475
								 square_t::E7,
								 piece_t::rook,
								 piece_t::empty,
								 square_t::F8);

			moves[6] = pack_move(piece_t::rook,   // 400
								 square_t::E7,
								 piece_t::pawn,
								 piece_t::empty,
								 square_t::F8);

			moves[5] = pack_move(piece_t::knight, // 225
								 square_t::E7,
								 piece_t::pawn,
								 piece_t::empty,
								 square_t::F8);

			moves[4] = pack_move(piece_t::rook,   // 175
								 square_t::E7,
								 piece_t::knight,
								 piece_t::empty,
								 square_t::F8);

			moves[3] = pack_move(piece_t::rook,   // 0
								 square_t::E7,
								 piece_t::rook,
								 piece_t::empty,
								 square_t::F8);

			moves[2] = pack_move(piece_t::empty,  // -100
								 square_t::E7,
								 piece_t::pawn,
								 piece_t::empty,
								 square_t::F8);

			moves[1] = pack_move(piece_t::pawn,   // -400
								 square_t::E7,
								 piece_t::rook,
								 piece_t::empty,
								 square_t::F8);

			moves[0] = pack_move(piece_t::pawn,   // -875
								 square_t::E7,
								 piece_t::queen,
								 piece_t::empty,
								 square_t::F8);

			int32 move0 = moves[0];
			int32 move1 = moves[1];
			int32 move2 = moves[2];
			int32 move3 = moves[3];
			int32 move4 = moves[4];
			int32 move5 = moves[5];
			int32 move6 = moves[6];
			int32 move7 = moves[7];
			int32 move8 = moves[8];
			int32 move9 = moves[9];

			SelectionSort sort;
			sort.init( moves, 10 );

			int32 next;
			EXPECT_TRUE (sort.next(next, cmp_fun));
			EXPECT_EQ(next, move9);

			next = 0;
			EXPECT_TRUE (sort.next(next, cmp_fun));
			EXPECT_EQ(next, move8);

			next = 0;
			EXPECT_TRUE (sort.next(next, cmp_fun));
			EXPECT_EQ(next, move7);

			next = 0;
			EXPECT_TRUE (sort.next(next, cmp_fun));
			EXPECT_EQ(next, move6);

			next = 0;
			EXPECT_TRUE (sort.next(next, cmp_fun));
			EXPECT_EQ(next, move5);

			next = 0;
			EXPECT_TRUE (sort.next(next, cmp_fun));
			EXPECT_EQ(next, move4);

			next = 0;
			EXPECT_TRUE (sort.next(next, cmp_fun));
			EXPECT_EQ(next, move3);

			next = 0;
			EXPECT_TRUE (sort.next(next, cmp_fun));
			EXPECT_EQ(next, move2);

			next = 0;
			EXPECT_TRUE (sort.next(next, cmp_fun));
			EXPECT_EQ(next, move1);

			next = 0;
			EXPECT_TRUE (sort.next(next, cmp_fun));
			EXPECT_EQ(next, move0);
		}
	}
}
