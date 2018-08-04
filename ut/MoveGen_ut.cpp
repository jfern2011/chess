#include "gtest/gtest.h"

/*
 * Note: Include *after* gtest to protect against
 *       name conflicts with abort macros
 */

#include "src/MoveGen4.h"

using namespace Chess;

namespace
{
	TEST(MoveGen, promotions)
	{
		Handle<std::ostream>
			stream(new std::ostream(std::cout.rdbuf()));

		Position pos(stream);

		ASSERT_TRUE(pos.reset(
			"1b1n1r1q/P1P1P1P1/8/8/8/7k/8/7K w - - 0 1"));

		const square_t from[]       =  {square_t::A7,
										square_t::C7,
										square_t::E7,
										square_t::G7};

		const square_t to_capture[] =  {square_t::B8,
										square_t::D8,
										square_t::F8,
										square_t::H8};

		const square_t to_advance[] =  {square_t::A8,
										square_t::C8,
										square_t::E8,
										square_t::G8};

		const piece_t captured[]    =  {piece_t::bishop,
										piece_t::knight,
										piece_t::rook,
										piece_t::queen};

		const piece_t promote[]     =  {piece_t::bishop,
										piece_t::knight,
										piece_t::rook,
										piece_t::queen};

		std::vector<int32> expected;

		/*
		 * Add right-hand captures
		 */
		for (int i = 0; i < 4; i++)
		{
			for (int j = 0; j < 4; j++)
			{
				expected.push_back(pack_move(captured[i],
											 from[i],
											 piece_t::pawn,
											 promote[j],
											 to_capture[i]));
			}
		}

		/*
		 * Add left-hand captures
		 */
		for (int i = 1; i < 4; i++)
		{
			for (int j = 0; j < 4; j++)
			{
				expected.push_back(pack_move(captured[i-1],
											 from[i],
											 piece_t::pawn,
											 promote[j],
											 to_capture[i]));
			}
		}

		/*
		 * Add advances
		 */
		for (int i = 0; i < 4; i++)
		{
			for (int j = 0; j < 4; j++)
			{
				expected.push_back(pack_move(piece_t::empty,
											 from[i],
											 piece_t::pawn,
											 promote[j],
											 to_advance[i]));
			}
		}
		
		int32 actual[max_moves];

		size_t n_moves = MoveGen::generate_captures(pos, actual);

		std::string moves_str;
		for (size_t i = 0; i < n_moves; i++)
			moves_str += format_san(actual[i], "") + "\n";

		EXPECT_EQ(n_moves, expected.size())
			<< moves_str;
	}

	TEST(MoveGen, pawn_captures)
	{
		Handle<std::ostream>
			stream(new std::ostream(std::cout.rdbuf()));

		Position pos(stream);

		ASSERT_TRUE(pos.reset(
			"k7/8/K4p2/2PpP1P1/8/b1n1r1q1/1P1P1P1P/8 w - d6 0 1"));

		const square_t from_left[]  =  {square_t::B2,
										square_t::D2,
										square_t::F2,
										square_t::H2,
										square_t::G5};

		const square_t from_right[] =  {square_t::B2,
										square_t::D2,
										square_t::F2,
										square_t::E5};

		const square_t to_left[]    =  {square_t::A3,
										square_t::C3,
										square_t::E3,
										square_t::G3,
										square_t::F6};

		const square_t to_right[]   =  {square_t::C3,
										square_t::E3,
										square_t::G3,
										square_t::F6};

		const piece_t captured[]    =  {piece_t::bishop,
										piece_t::knight,
										piece_t::rook,
										piece_t::queen,
										piece_t::pawn};

		std::vector<int32> expected;

		/*
		 * Add right-hand captures
		 */
		for (int i = 0; i < 4; i++)
		{
			expected.push_back(pack_move(captured[i+1],
										 from_right[i],
										 piece_t::pawn,
										 piece_t::empty,
										 to_right[i]));
		}

		/*
		 * Add left-hand captures
		 */
		for (int i = 0; i < 5; i++)
		{
			expected.push_back(pack_move(captured[i],
										 from_left[i],
										 piece_t::pawn,
										 piece_t::empty,
										 to_left[i]));
		}

		/*
		 * Add en passant captures
		 */
		expected.push_back(pack_move(piece_t::pawn,
									 square_t::C5,
									 piece_t::pawn,
									 piece_t::empty,
									 square_t::D6));

		expected.push_back(pack_move(piece_t::pawn,
									 square_t::E5,
									 piece_t::pawn,
									 piece_t::empty,
									 square_t::D6));
		
		int32 actual[max_moves];

		size_t n_moves = MoveGen::generate_captures(pos, actual);

		std::string moves_str;
		for (size_t i = 0; i < n_moves; i++)
			moves_str += format_san(actual[i], "") + "\n";

		EXPECT_EQ(n_moves, expected.size())
			<< moves_str;
	}

	TEST(MoveGen, pawn_advances)
	{
		Handle<std::ostream>
			stream(new std::ostream(std::cout.rdbuf()));

		Position pos(stream);

		ASSERT_TRUE(pos.reset(
			"6r1/8/3P4/8/8/7k/2P5/7K w - - 0 1"));

		int32 actual[max_moves];

		size_t n_moves = MoveGen::generate_noncaptures(pos, actual);

		std::vector<int32> expected;

		expected.push_back(pack_move(piece_t::empty,
									 square_t::D6,
									 piece_t::pawn,
									 piece_t::empty,
									 square_t::D7));

		expected.push_back(pack_move(piece_t::empty,
									 square_t::C2,
									 piece_t::pawn,
									 piece_t::empty,
									 square_t::C3));

		expected.push_back(pack_move(piece_t::empty,
									 square_t::C2,
									 piece_t::pawn,
									 piece_t::empty,
									 square_t::C4));

		std::string moves_str;
		for (size_t i = 0; i < n_moves; i++)
			moves_str += format_san(actual[i], "") + "\n";

		EXPECT_EQ(n_moves, expected.size())
			<< moves_str;
	}

	TEST(MoveGen, castling)
	{
		Handle<std::ostream>
			stream(new std::ostream(std::cout.rdbuf()));

		Position pos(stream);

		{
			ASSERT_TRUE(pos.reset(
				"r3k2r/PP4PP/8/8/8/p6p/P6P/R3K2R w KQkq - 0 1"));

			int32 actual[max_moves];

			size_t n_moves = MoveGen::generate_noncaptures(pos, actual);

			const square_t to_king[]    =  {square_t::C1,
											square_t::G1,
											square_t::D1,
											square_t::F1,
											square_t::D2,
											square_t::E2,
											square_t::F2};

			std::vector<int32> expected;

			expected.push_back(pack_move(piece_t::empty,
										 square_t::A1,
										 piece_t::rook,
										 piece_t::empty,
										 square_t::B1));

			expected.push_back(pack_move(piece_t::empty,
										 square_t::A1,
										 piece_t::rook,
										 piece_t::empty,
										 square_t::C1));

			expected.push_back(pack_move(piece_t::empty,
										 square_t::A1,
										 piece_t::rook,
										 piece_t::empty,
										 square_t::D1));

			expected.push_back(pack_move(piece_t::empty,
										 square_t::H1,
										 piece_t::rook,
										 piece_t::empty,
										 square_t::G1));

			expected.push_back(pack_move(piece_t::empty,
										 square_t::H1,
										 piece_t::rook,
										 piece_t::empty,
										 square_t::F1));

			for (int i = 0; i < 7; i++)
			{
				expected.push_back(pack_move(piece_t::empty,
											 square_t::E1,
											 piece_t::king,
											 piece_t::empty,
											 to_king[i]));
			}

			std::string moves_str;
			for (size_t i = 0; i < n_moves; i++)
				moves_str += format_san(actual[i], "") + "\n";

			EXPECT_EQ(n_moves, expected.size())
				<< moves_str;
		}

		{
			ASSERT_TRUE(pos.reset(
				"r3k2r/PP4PP/8/8/8/p6p/P6P/R3K2R b KQkq - 0 1"));

			int32 actual[max_moves];

			size_t n_moves = MoveGen::generate_noncaptures(pos, actual);

			const square_t to_king[]    =  {square_t::D8,
											square_t::D7,
											square_t::E7,
											square_t::F7};

			std::vector<int32> expected;

			expected.push_back(pack_move(piece_t::empty,
										 square_t::A8,
										 piece_t::rook,
										 piece_t::empty,
										 square_t::B8));

			expected.push_back(pack_move(piece_t::empty,
										 square_t::A8,
										 piece_t::rook,
										 piece_t::empty,
										 square_t::C8));

			expected.push_back(pack_move(piece_t::empty,
										 square_t::A8,
										 piece_t::rook,
										 piece_t::empty,
										 square_t::D8));

			expected.push_back(pack_move(piece_t::empty,
										 square_t::H8,
										 piece_t::rook,
										 piece_t::empty,
										 square_t::G8));

			expected.push_back(pack_move(piece_t::empty,
										 square_t::H8,
										 piece_t::rook,
										 piece_t::empty,
										 square_t::F8));

			for (int i = 0; i < 4; i++)
			{
				expected.push_back(pack_move(piece_t::empty,
											 square_t::E8,
											 piece_t::king,
											 piece_t::empty,
											 to_king[i]));
			}

			std::string moves_str;
			for (size_t i = 0; i < n_moves; i++)
				moves_str += format_san(actual[i], "") + "\n";

			EXPECT_EQ(n_moves, expected.size())
				<< moves_str;
		}
	}

	TEST(MoveGen, king)
	{
		Handle<std::ostream>
			stream(new std::ostream(std::cout.rdbuf()));

		Position pos(stream);

		{
			ASSERT_TRUE(pos.reset(
				"4k3/8/8/4K3/8/8/8/8 w - - 0 1"));

			int32 actual[max_moves];

			size_t n_moves = MoveGen::generate_noncaptures(pos, actual);

			const square_t to_king[]    =  {square_t::F4,
											square_t::E4,
											square_t::D4,
											square_t::F5,
											square_t::D5,
											square_t::F6,
											square_t::E6,
											square_t::D6};

			std::vector<int32> expected;

			for (int i = 0; i < 8; i++)
			{
				expected.push_back(pack_move(piece_t::empty,
											 square_t::E5,
											 piece_t::king,
											 piece_t::empty,
											 to_king[i]));
			}

			std::string moves_str;
			for (size_t i = 0; i < n_moves; i++)
				moves_str += format_san(actual[i], "") + "\n";

			EXPECT_EQ(n_moves, expected.size())
				<< moves_str;
		}

		{
			ASSERT_TRUE(pos.reset(
				"4k3/8/3ppp2/4K3/3ppp2/8/8/8 w - - 0 1"));

			int32 actual[max_moves];

			size_t n_moves = MoveGen::generate_captures(pos, actual);

			const square_t to_king[]    =  {square_t::F4,
											square_t::E4,
											square_t::D4,
											square_t::F6,
											square_t::E6,
											square_t::D6};

			std::vector<int32> expected;

			for (int i = 0; i < 6; i++)
			{
				expected.push_back(pack_move(piece_t::pawn,
											 square_t::E5,
											 piece_t::king,
											 piece_t::empty,
											 to_king[i]));
			}

			std::string moves_str;
			for (size_t i = 0; i < n_moves; i++)
				moves_str += format_san(actual[i], "") + "\n";

			EXPECT_EQ(n_moves, expected.size())
				<< moves_str;
		}

		{
			ASSERT_TRUE(pos.reset(
				"4k3/8/8/3pKp1r/8/8/8/8 w - - 0 1"));

			int32 actual[max_moves];

			size_t n_moves = MoveGen::generate_captures(pos, actual);

			std::vector<int32> expected;
			
			expected.push_back(pack_move(piece_t::pawn,
										 square_t::E5,
										 piece_t::king,
										 piece_t::empty,
										 square_t::D5));

			std::string moves_str;
			for (size_t i = 0; i < n_moves; i++)
				moves_str += format_san(actual[i], "") + "\n";

			EXPECT_EQ(n_moves, expected.size())
				<< moves_str;
		}
	}
}
