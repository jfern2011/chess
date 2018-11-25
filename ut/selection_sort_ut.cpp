#include "gtest/gtest.h"
#include "src/selection_sort.h"
#include "src/chess4.h"

using namespace Chess;

namespace
{
	TEST(selection_sort, one_item)
	{
		int32 move = pack_move(piece_t::rook,
							   square_t::E7,
							   piece_t::pawn,
							   piece_t::empty,
							   square_t::F8);

		SelectionSort sort(&move, 1);

		EXPECT_EQ(sort.next(), move);
		EXPECT_EQ(sort.next(), move);
		EXPECT_EQ(sort.next(), move);
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

			SelectionSort sort(moves, 2);

			EXPECT_EQ(sort.next(), move0);
			EXPECT_EQ(sort.next(), move1);
			EXPECT_EQ(sort.next(), move1);
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

			SelectionSort sort(moves, 2);

			EXPECT_EQ(sort.next(), move1);
			EXPECT_EQ(sort.next(), move0);
			EXPECT_EQ(sort.next(), move0);
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

			SelectionSort sort(moves, 10);

			EXPECT_EQ(sort.next(), move9);
			EXPECT_EQ(sort.next(), move8);
			EXPECT_EQ(sort.next(), move7);
			EXPECT_EQ(sort.next(), move6);
			EXPECT_EQ(sort.next(), move5);
			EXPECT_EQ(sort.next(), move4);
			EXPECT_EQ(sort.next(), move3);
			EXPECT_EQ(sort.next(), move2);
			EXPECT_EQ(sort.next(), move1);
			EXPECT_EQ(sort.next(), move0);
			EXPECT_EQ(sort.next(), move0);
		}
	}
}
