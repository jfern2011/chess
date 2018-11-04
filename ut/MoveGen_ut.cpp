#include "gtest/gtest.h"

/*
 * Note: Include *after* gtest to protect against
 *       name conflicts with abort macros
 */

#include "src/MoveGen4.h"

using namespace Chess;

namespace
{
	/*
	 * Returns true if a move is in a list of moves
	 */
	bool in_move_list(int32 move, const int32* list, size_t size)
	{
		for (size_t i = 0; i < size; i++)
		{
			if (list[i] == move) return true;
		}
		return false;
	}

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

		/* Special case for en passant: */

		ASSERT_TRUE(pos.reset(
			"4k3/8/8/2KPp1r1/8/8/8/8 w - e6 0 1"));

		n_moves = MoveGen::generate_captures(pos, actual);

		moves_str.clear();
		for (size_t i = 0; i < n_moves; i++)
			moves_str += format_san(actual[i], "") + "\n";

		ASSERT_EQ(n_moves, 0) << moves_str;
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

	TEST(MoveGen, rook)
	{
		Handle<std::ostream>
			stream(new std::ostream(std::cout.rdbuf()));

		Position pos(stream);

		{
			ASSERT_TRUE(pos.reset(
				"6K1/8/4pq1r/8/3pR3/8/8/4b1k1 w - - 0 1"));

			int32 actual[max_moves];

			size_t n_moves = MoveGen::generate_noncaptures(pos, actual);

			const square_t to_rook[]    =  {square_t::E2,
											square_t::E3,
											square_t::E5,
											square_t::F4,
											square_t::G4,
											square_t::H4};

			std::vector<int32> expected;

			for (int i = 0; i < 6; i++)
			{
				expected.push_back(pack_move(piece_t::empty,
											 square_t::E4,
											 piece_t::rook,
											 piece_t::empty,
											 to_rook[i]));
			}

			std::string moves_str;
			for (size_t i = 0; i < n_moves; i++)
				moves_str += format_san(actual[i], "") + "\n";

			EXPECT_EQ(n_moves, expected.size())
				<< moves_str;
		}

		{
			ASSERT_TRUE(pos.reset(
				"6K1/8/4pq1r/8/3pR3/8/8/4b1k1 w - - 0 1"));

			int32 actual[max_moves];

			size_t n_moves = MoveGen::generate_captures(pos, actual);

			const square_t to_rook[]    =  {square_t::E1,
											square_t::D4,
											square_t::E6};

			const piece_t captured[]    =  {piece_t::pawn,
											piece_t::pawn,
											piece_t::bishop};

			std::vector<int32> expected;

			for (int i = 0; i < 3; i++)
			{
				expected.push_back(pack_move(captured[i],
											 square_t::E4,
											 piece_t::rook,
											 piece_t::empty,
											 to_rook[i]));
			}

			std::string moves_str;
			for (size_t i = 0; i < n_moves; i++)
				moves_str += format_san(actual[i], "") + "\n";

			EXPECT_EQ(n_moves, expected.size())
				<< moves_str;
		}
	}

	TEST(MoveGen, knight)
	{
		Handle<std::ostream>
			stream(new std::ostream(std::cout.rdbuf()));

		Position pos(stream);

		{
			ASSERT_TRUE(pos.reset(
				"7K/3pr1r1/6p1/4N3/2p5/5p2/8/7k w - - 0 1"));

			int32 actual[max_moves];

			size_t n_moves = MoveGen::generate_noncaptures(pos, actual);

			const square_t to_knight[]  =  {square_t::F7,
											square_t::C6,
											square_t::G4,
											square_t::D3};

			std::vector<int32> expected;

			for (int i = 0; i < 4; i++)
			{
				expected.push_back(pack_move(piece_t::empty,
											 square_t::E5,
											 piece_t::knight,
											 piece_t::empty,
											 to_knight[i]));
			}

			std::string moves_str;
			for (size_t i = 0; i < n_moves; i++)
				moves_str += format_san(actual[i], "") + "\n";

			EXPECT_EQ(n_moves, expected.size())
				<< moves_str;
		}

		{
			ASSERT_TRUE(pos.reset(
				"7K/3pr1r1/6p1/4N3/2p5/5p2/8/7k w - - 0 1"));

			int32 actual[max_moves];

			size_t n_moves = MoveGen::generate_captures(pos, actual);

			const square_t to_knight[]  =  {square_t::G6,
											square_t::D7,
											square_t::C4,
											square_t::F3};

			std::vector<int32> expected;

			for (int i = 0; i < 4; i++)
			{
				expected.push_back(pack_move(piece_t::pawn,
											 square_t::E5,
											 piece_t::knight,
											 piece_t::empty,
											 to_knight[i]));
			}

			std::string moves_str;
			for (size_t i = 0; i < n_moves; i++)
				moves_str += format_san(actual[i], "") + "\n";

			EXPECT_EQ(n_moves, expected.size())
				<< moves_str;
		}
	}

	TEST(MoveGen, bishop)
	{
		Handle<std::ostream>
			stream(new std::ostream(std::cout.rdbuf()));

		Position pos(stream);

		{
			ASSERT_TRUE(pos.reset(
				"7K/p3r1r1/3p4/2B5/8/8/8/6bk w - - 0 1"));

			int32 actual[max_moves];

			size_t n_moves = MoveGen::generate_noncaptures(pos, actual);

			const square_t to_bishop[]  =  {square_t::B6,
											square_t::A3,
											square_t::B4,
											square_t::F2,
											square_t::E3,
											square_t::D4};

			std::vector<int32> expected;

			for (int i = 0; i < 6; i++)
			{
				expected.push_back(pack_move(piece_t::empty,
											 square_t::C5,
											 piece_t::bishop,
											 piece_t::empty,
											 to_bishop[i]));
			}

			std::string moves_str;
			for (size_t i = 0; i < n_moves; i++)
				moves_str += format_san(actual[i], "") + "\n";

			EXPECT_EQ(n_moves, expected.size())
				<< moves_str;
		}

		{
			ASSERT_TRUE(pos.reset(
				"7K/p3r1r1/3p4/2B5/8/8/8/6bk w - - 0 1"));

			int32 actual[max_moves];

			size_t n_moves = MoveGen::generate_captures(pos, actual);

			const square_t to_bishop[]  =  {square_t::A7,
											square_t::D6,
											square_t::G1};

			const piece_t captured[]    =  {piece_t::pawn,
											piece_t::pawn,
											piece_t::bishop};

			std::vector<int32> expected;

			for (int i = 0; i < 3; i++)
			{
				expected.push_back(pack_move(captured[i],
											 square_t::C5,
											 piece_t::bishop,
											 piece_t::empty,
											 to_bishop[i]));
			}

			std::string moves_str;
			for (size_t i = 0; i < n_moves; i++)
				moves_str += format_san(actual[i], "") + "\n";

			EXPECT_EQ(n_moves, expected.size())
				<< moves_str;
		}
	}

	TEST(MoveGen, evasions)
	{
		Handle<std::ostream>
			stream(new std::ostream(std::cout.rdbuf()));

		Position pos(stream);

		{
			ASSERT_TRUE(pos.reset(
				"4k3/8/8/3q4/4K3/5r2/8/8 w - - 0 1"));

			int32 actual[max_moves];

			size_t n_moves = MoveGen::generate_check_evasions(pos, actual);

			std::string moves_str;
			for (size_t i = 0; i < n_moves; i++)
				moves_str += format_san(actual[i], "") + "\n";

			ASSERT_EQ(n_moves, 1) << moves_str;

			ASSERT_EQ(actual[0], pack_move(piece_t::queen,
										   square_t::E4,
										   piece_t::king,
										   piece_t::empty,
										   square_t::D5));
		}

		{
			ASSERT_TRUE(pos.reset(
				"B3k1B1/1b5R/P6R/2N5/3PK3/1QN5/8/8 w - - 0 1"));

			int32 actual[max_moves];

			size_t n_moves = MoveGen::generate_check_evasions(pos, actual);

			std::vector<int32> expected;

			const square_t from[]   =  {square_t::A6,
										square_t::D4,
										square_t::C3,
										square_t::C5,
										square_t::A8,
										square_t::G8,
										square_t::H6,
										square_t::H7,
										square_t::B3,
										square_t::B3,
										square_t::E4,
										square_t::E4,
										square_t::E4,
										square_t::E4,
										square_t::E4};

			const square_t to[]     =  {square_t::B7,
										square_t::D5,
										square_t::D5,
										square_t::B7,
										square_t::B7,
										square_t::D5,
										square_t::C6,
										square_t::B7,
										square_t::B7,
										square_t::D5,
										square_t::D3,
										square_t::E3,
										square_t::F4,
										square_t::E5,
										square_t::F5};

			const piece_t moved[]   =  {piece_t::pawn,
										piece_t::pawn,
										piece_t::knight,
										piece_t::knight,
										piece_t::bishop,
										piece_t::bishop,
										piece_t::rook,
										piece_t::rook,
										piece_t::queen,
										piece_t::queen,
										piece_t::king,
										piece_t::king,
										piece_t::king,
										piece_t::king,
										piece_t::king};

			const piece_t captured[] = {piece_t::bishop,
									    piece_t::empty,
									    piece_t::empty,
									    piece_t::bishop,
									    piece_t::bishop,
									    piece_t::empty,
									    piece_t::empty,
									    piece_t::bishop,
									    piece_t::bishop,
									    piece_t::empty,
									    piece_t::empty,
									    piece_t::empty,
									    piece_t::empty,
									    piece_t::empty,
									    piece_t::empty};

			for (int i = 0; i < 15; i++)
			{
				expected.push_back(pack_move(captured[i],
											 from[i],
											 moved[i],
											 piece_t::empty,
											 to[i]));
			}

			std::string moves_str;
			for (size_t i = 0; i < n_moves; i++)
				moves_str += format_san(actual[i], "") + "\n";

			EXPECT_EQ(n_moves, expected.size())
				<< moves_str;

			for (size_t i = 0; i < expected.size(); i++)
			{
				EXPECT_TRUE( in_move_list(expected[i], actual,
					n_moves)) << moves_str;
			}
		}

		{
			ASSERT_TRUE(pos.reset(
				"4k3/8/8/3q4/4Kr2/8/3Q4/8 w - - 0 1"));

			int32 actual[max_moves];

			size_t n_moves = MoveGen::generate_check_evasions(pos, actual);

			std::vector<int32> expected;

			expected.push_back(pack_move(piece_t::rook,
										 square_t::E4,
										 piece_t::king,
										 piece_t::empty,
										 square_t::F4));

			expected.push_back(pack_move(piece_t::queen,
										 square_t::E4,
										 piece_t::king,
										 piece_t::empty,
										 square_t::D5));

			expected.push_back(pack_move(piece_t::empty,
										 square_t::E4,
										 piece_t::king,
										 piece_t::empty,
										 square_t::E3));

			std::string moves_str;
			for (size_t i = 0; i < n_moves; i++)
				moves_str += format_san(actual[i], "") + "\n";

			EXPECT_EQ(n_moves, expected.size())
				<< moves_str;

			for (size_t i = 0; i < expected.size(); i++)
			{
				EXPECT_TRUE( in_move_list(expected[i], actual,
					n_moves)) << moves_str;
			}
		}

		{
			ASSERT_TRUE(pos.reset(
				"4k2b/8/8/3PpP2/5K2/8/8/8 w - e6 0 1"));

			int32 actual[max_moves];

			size_t n_moves = MoveGen::generate_check_evasions(pos, actual);

			std::vector<int32> expected;

			expected.push_back(pack_move(piece_t::pawn,
										 square_t::D5,
										 piece_t::pawn,
										 piece_t::empty,
										 square_t::E6));

			expected.push_back(pack_move(piece_t::pawn,
										 square_t::F5,
										 piece_t::pawn,
										 piece_t::empty,
										 square_t::E6));

			const square_t king_to[] = {square_t::G5,
										square_t::E4,
										square_t::G4,
										square_t::E3,
										square_t::F3,
										square_t::G3};

			for (int i = 0; i < 6; i++)
			{
				expected.push_back(pack_move(piece_t::empty,
											 square_t::F4,
											 piece_t::king,
											 piece_t::empty,
											 king_to[i]));
			}

			std::string moves_str;
			for (size_t i = 0; i < n_moves; i++)
				moves_str += format_san(actual[i], "") + "\n";

			ASSERT_EQ(n_moves, expected.size())
				<< moves_str;

			for (size_t i = 0; i < expected.size(); i++)
			{
				ASSERT_TRUE( in_move_list(expected[i], actual,
					n_moves)) << moves_str;
			}
		}

		{
			ASSERT_TRUE(pos.reset(
				"4k2b/5r2/8/3PpP2/5K2/8/8/8 w - e6 0 1"));

			int32 actual[max_moves];

			size_t n_moves = MoveGen::generate_check_evasions(pos, actual);

			std::vector<int32> expected;

			expected.push_back(pack_move(piece_t::pawn,
										 square_t::D5,
										 piece_t::pawn,
										 piece_t::empty,
										 square_t::E6));

			const square_t king_to[] = {square_t::G5,
										square_t::E4,
										square_t::G4,
										square_t::E3,
										square_t::F3,
										square_t::G3};

			for (int i = 0; i < 6; i++)
			{
				expected.push_back(pack_move(piece_t::empty,
											 square_t::F4,
											 piece_t::king,
											 piece_t::empty,
											 king_to[i]));
			}

			std::string moves_str;
			for (size_t i = 0; i < n_moves; i++)
				moves_str += format_san(actual[i], "") + "\n";

			ASSERT_EQ(n_moves, expected.size())
				<< moves_str;

			for (size_t i = 0; i < expected.size(); i++)
			{
				ASSERT_TRUE( in_move_list(expected[i], actual,
					n_moves)) << moves_str;
			}
		}

		{
			ASSERT_TRUE(pos.reset(
				"2r4K/6P1/2k5/8/8/8/8/8 w - - 0 1"));

			int32 actual[max_moves];

			size_t n_moves = MoveGen::generate_check_evasions(pos, actual);

			std::vector<int32> expected;

			expected.push_back(pack_move(piece_t::empty,
										 square_t::H8,
										 piece_t::king,
										 piece_t::empty,
										 square_t::H7));

			const piece_t promote[] =  {piece_t::knight,
										piece_t::bishop,
										piece_t::rook,
										piece_t::queen};

			for (int i = 0; i < 4; i++)
			{
				expected.push_back(pack_move(piece_t::empty,
											 square_t::G7,
											 piece_t::pawn,
											 promote[i],
											 square_t::G8));
			}

			std::string moves_str;
			for (size_t i = 0; i < n_moves; i++)
				moves_str += format_san(actual[i], "") + "\n";

			ASSERT_EQ(n_moves, expected.size())
				<< moves_str;

			for (size_t i = 0; i < expected.size(); i++)
			{
				ASSERT_TRUE( in_move_list(expected[i], actual,
					n_moves)) << moves_str;
			}
		}

		{
			ASSERT_TRUE(pos.reset(
				"8/8/2k5/8/2r4K/8/6P1/8 w - - 0 1"));

			int32 actual[max_moves];

			size_t n_moves = MoveGen::generate_check_evasions(pos, actual);

			std::vector<int32> expected;

			expected.push_back(pack_move(piece_t::empty,
										 square_t::G2,
										 piece_t::pawn,
										 piece_t::empty,
										 square_t::G4));

			const square_t king_to[] =  {square_t::H5,
										 square_t::G5,
										 square_t::G3,
										 square_t::H3};

			for (int i = 0; i < 4; i++)
			{
				expected.push_back(pack_move(piece_t::empty,
											 square_t::H4,
											 piece_t::king,
											 piece_t::empty,
											 king_to[i]));
			}

			std::string moves_str;
			for (size_t i = 0; i < n_moves; i++)
				moves_str += format_san(actual[i], "") + "\n";

			ASSERT_EQ(n_moves, expected.size())
				<< moves_str;

			for (size_t i = 0; i < expected.size(); i++)
			{
				ASSERT_TRUE( in_move_list(expected[i], actual,
					n_moves)) << moves_str;
			}
		}
	}

	TEST(MoveGen, checks)
	{
		Handle<std::ostream>
			stream(new std::ostream(std::cout.rdbuf()));

		Position pos(stream);

		{
			ASSERT_TRUE(pos.reset(
				"8/8/8/8/7k/8/7N/6KR w - - 0 1"));

			std::vector<int32> expected;

			expected.push_back(pack_move(piece_t::empty,
										 square_t::H2,
										 piece_t::knight,
										 piece_t::empty,
										 square_t::G4));

			expected.push_back(pack_move(piece_t::empty,
										 square_t::H2,
										 piece_t::knight,
										 piece_t::empty,
										 square_t::F3));

			expected.push_back(pack_move(piece_t::empty,
										 square_t::H2,
										 piece_t::knight,
										 piece_t::empty,
										 square_t::F1));
			
			int32 actual[max_moves];

			size_t n_moves = MoveGen::generate_checks(pos, actual);

			std::string moves_str;
			for (size_t i = 0; i < n_moves; i++)
				moves_str += format_san(actual[i], "") + "\n";

			EXPECT_EQ(n_moves, expected.size())
				<< moves_str;
		}

		{
			ASSERT_TRUE(pos.reset(
				"8/8/8/8/8/8/1k2P2R/4K3 w - - 0 1"));

			std::vector<int32> expected;

			expected.push_back(pack_move(piece_t::empty,
										 square_t::E2,
										 piece_t::pawn,
										 piece_t::empty,
										 square_t::E3));

			expected.push_back(pack_move(piece_t::empty,
										 square_t::E2,
										 piece_t::pawn,
										 piece_t::empty,
										 square_t::E4));
			
			int32 actual[max_moves];

			size_t n_moves = MoveGen::generate_checks(pos, actual);

			std::string moves_str;
			for (size_t i = 0; i < n_moves; i++)
				moves_str += format_san(actual[i], "") + "\n";

			EXPECT_EQ(n_moves, expected.size())
				<< moves_str;
		}

		{
			ASSERT_TRUE(pos.reset(
				"8/8/8/8/3k4/8/4P3/4K3 w - - 0 1"));

			std::vector<int32> expected;

			expected.push_back(pack_move(piece_t::empty,
										 square_t::E2,
										 piece_t::pawn,
										 piece_t::empty,
										 square_t::E3));
			
			int32 actual[max_moves];

			size_t n_moves = MoveGen::generate_checks(pos, actual);

			std::string moves_str;
			for (size_t i = 0; i < n_moves; i++)
				moves_str += format_san(actual[i], "") + "\n";

			EXPECT_EQ(n_moves, expected.size())
				<< moves_str;
		}

		{
			ASSERT_TRUE(pos.reset(
				"8/8/8/5k2/8/8/4P3/4K3 w - - 0 1"));

			std::vector<int32> expected;

			expected.push_back(pack_move(piece_t::empty,
										 square_t::E2,
										 piece_t::pawn,
										 piece_t::empty,
										 square_t::E4));
			
			int32 actual[max_moves];

			size_t n_moves = MoveGen::generate_checks(pos, actual);

			std::string moves_str;
			for (size_t i = 0; i < n_moves; i++)
				moves_str += format_san(actual[i], "") + "\n";

			EXPECT_EQ(n_moves, expected.size())
				<< moves_str;
		}

		{
			ASSERT_TRUE(pos.reset(
				"8/8/k7/8/8/8/4N3/5B1K w - - 0 1"));

			std::vector<int32> expected;

			expected.push_back(pack_move(piece_t::empty,
										 square_t::E2,
										 piece_t::knight,
										 piece_t::empty,
										 square_t::G1));

			expected.push_back(pack_move(piece_t::empty,
										 square_t::E2,
										 piece_t::knight,
										 piece_t::empty,
										 square_t::C1));

			expected.push_back(pack_move(piece_t::empty,
										 square_t::E2,
										 piece_t::knight,
										 piece_t::empty,
										 square_t::G3));

			expected.push_back(pack_move(piece_t::empty,
										 square_t::E2,
										 piece_t::knight,
										 piece_t::empty,
										 square_t::C3));

			expected.push_back(pack_move(piece_t::empty,
										 square_t::E2,
										 piece_t::knight,
										 piece_t::empty,
										 square_t::D4));

			expected.push_back(pack_move(piece_t::empty,
										 square_t::E2,
										 piece_t::knight,
										 piece_t::empty,
										 square_t::F4));

			int32 actual[max_moves];

			size_t n_moves = MoveGen::generate_checks(pos, actual);

			std::string moves_str;
			for (size_t i = 0; i < n_moves; i++)
				moves_str += format_san(actual[i], "") + "\n";

			EXPECT_EQ(n_moves, expected.size())
				<< moves_str;
		}

		{
			ASSERT_TRUE(pos.reset(
				"8/8/8/1k6/8/8/4N3/7K w - - 0 1"));

			std::vector<int32> expected;

			expected.push_back(pack_move(piece_t::empty,
										 square_t::E2,
										 piece_t::knight,
										 piece_t::empty,
										 square_t::C3));

			expected.push_back(pack_move(piece_t::empty,
										 square_t::E2,
										 piece_t::knight,
										 piece_t::empty,
										 square_t::D4));
			
			int32 actual[max_moves];

			size_t n_moves = MoveGen::generate_checks(pos, actual);

			std::string moves_str;
			for (size_t i = 0; i < n_moves; i++)
				moves_str += format_san(actual[i], "") + "\n";

			EXPECT_EQ(n_moves, expected.size())
				<< moves_str;
		}

		{
			ASSERT_TRUE(pos.reset(
				"8/8/4k3/8/4N3/8/8/4R2K w - - 0 1"));

			std::vector<int32> expected;

			expected.push_back(pack_move(piece_t::empty,
										 square_t::E4,
										 piece_t::knight,
										 piece_t::empty,
										 square_t::F2));

			expected.push_back(pack_move(piece_t::empty,
										 square_t::E4,
										 piece_t::knight,
										 piece_t::empty,
										 square_t::D2));

			expected.push_back(pack_move(piece_t::empty,
										 square_t::E4,
										 piece_t::knight,
										 piece_t::empty,
										 square_t::C3));

			expected.push_back(pack_move(piece_t::empty,
										 square_t::E4,
										 piece_t::knight,
										 piece_t::empty,
										 square_t::G3));

			expected.push_back(pack_move(piece_t::empty,
										 square_t::E4,
										 piece_t::knight,
										 piece_t::empty,
										 square_t::C5));

			expected.push_back(pack_move(piece_t::empty,
										 square_t::E4,
										 piece_t::knight,
										 piece_t::empty,
										 square_t::G5));

			expected.push_back(pack_move(piece_t::empty,
										 square_t::E4,
										 piece_t::knight,
										 piece_t::empty,
										 square_t::D6));

			expected.push_back(pack_move(piece_t::empty,
										 square_t::E4,
										 piece_t::knight,
										 piece_t::empty,
										 square_t::F6));
			
			int32 actual[max_moves];

			size_t n_moves = MoveGen::generate_checks(pos, actual);

			std::string moves_str;
			for (size_t i = 0; i < n_moves; i++)
				moves_str += format_san(actual[i], "") + "\n";

			EXPECT_EQ(n_moves, expected.size())
				<< moves_str;
		}

		{
			ASSERT_TRUE(pos.reset(
				"7R/8/5P2/3k4/8/3p1R1P/6B1/5K2 w - - 0 1"));

			std::vector<int32> expected;

			expected.push_back(pack_move(piece_t::empty,
										 square_t::F3,
										 piece_t::rook,
										 piece_t::empty,
										 square_t::E3));

			expected.push_back(pack_move(piece_t::empty,
										 square_t::F3,
										 piece_t::rook,
										 piece_t::empty,
										 square_t::G3));

			expected.push_back(pack_move(piece_t::empty,
										 square_t::F3,
										 piece_t::rook,
										 piece_t::empty,
										 square_t::F2));

			expected.push_back(pack_move(piece_t::empty,
										 square_t::F3,
										 piece_t::rook,
										 piece_t::empty,
										 square_t::F4));

			expected.push_back(pack_move(piece_t::empty,
										 square_t::F3,
										 piece_t::rook,
										 piece_t::empty,
										 square_t::F5));

			expected.push_back(pack_move(piece_t::empty,
										 square_t::H8,
										 piece_t::rook,
										 piece_t::empty,
										 square_t::H5));

			expected.push_back(pack_move(piece_t::empty,
										 square_t::H8,
										 piece_t::rook,
										 piece_t::empty,
										 square_t::D8));
			
			int32 actual[max_moves];

			size_t n_moves = MoveGen::generate_checks(pos, actual);

			std::string moves_str;
			for (size_t i = 0; i < n_moves; i++)
				moves_str += format_san(actual[i], "") + "\n";

			EXPECT_EQ(n_moves, expected.size())
				<< moves_str;
		}

		{
			ASSERT_TRUE(pos.reset(
				"3K4/3Q4/8/8/8/8/7k/3r4 w - - 0 1"));

			std::vector<int32> expected;

			expected.push_back(pack_move(piece_t::empty,
										 square_t::D7,
										 piece_t::queen,
										 piece_t::empty,
										 square_t::D2));

			expected.push_back(pack_move(piece_t::empty,
										 square_t::D7,
										 piece_t::queen,
										 piece_t::empty,
										 square_t::D6));

			int32 actual[max_moves];

			size_t n_moves = MoveGen::generate_checks(pos, actual);

			std::string moves_str;
			for (size_t i = 0; i < n_moves; i++)
				moves_str += format_san(actual[i], "") + "\n";

			EXPECT_EQ(n_moves, expected.size())
				<< moves_str;
		}
	}

	TEST(MoveGen, validate_move)
	{
		Handle<std::ostream>
			stream(new std::ostream(std::cout.rdbuf()));

		Position pos(stream);

		{
			ASSERT_TRUE(pos.reset(
					"4k3/4r3/8/2p5/8/8/4NB2/4K3 w - - 0 1"));

			std::vector<int32> moves;

			moves.push_back(pack_move(piece_t::empty,
									  square_t::F2,
									  piece_t::bishop,
									  piece_t::empty,
									  square_t::B6));

			moves.push_back(pack_move(piece_t::empty,
									  square_t::E2,
									  piece_t::knight,
									  piece_t::empty,
									  square_t::F4));

			for (auto& move : moves)
			{
				EXPECT_FALSE(MoveGen::validate_move(
					pos, move, false));
			}
		}

		{
			ASSERT_TRUE(pos.reset(
					"4k3/4r3/8/2p5/8/8/4RB2/4K3 w - - 0 1"));

			std::vector<int32> moves;

			moves.push_back(pack_move(piece_t::rook,
									  square_t::E2,
									  piece_t::rook,
									  piece_t::empty,
									  square_t::E7));

			for (auto& move : moves)
			{
				EXPECT_TRUE(MoveGen::validate_move(
					pos, move, false));
			}
		}

		{
			ASSERT_TRUE(pos.reset(
					"4k3/7p/6B1/3n4/8/4R3/8/4K3 b - - 0 1"));

			std::vector<int32> moves;

			moves.push_back(pack_move(piece_t::rook,
									  square_t::D5,
									  piece_t::knight,
									  piece_t::empty,
									  square_t::E3));

			moves.push_back(pack_move(piece_t::bishop,
									  square_t::H7,
									  piece_t::pawn,
									  piece_t::empty,
									  square_t::G6));

			for (auto& move : moves)
			{
				EXPECT_FALSE(MoveGen::validate_move(
					pos, move, true));
			}
		}

		{
			ASSERT_TRUE(pos.reset(
					"4k3/7p/8/3n4/8/4R3/8/4K3 b - - 0 1"));

			std::vector<int32> moves;

			moves.push_back(pack_move(piece_t::rook,
									  square_t::D5,
									  piece_t::knight,
									  piece_t::empty,
									  square_t::E3));

			for (auto& move : moves)
			{
				EXPECT_TRUE(MoveGen::validate_move(
					pos, move, true));
			}
		}

		{
			ASSERT_TRUE(pos.reset(
					"4k3/7p/6B1/3n4/8/8/8/4K3 b - - 0 1"));

			std::vector<int32> moves;

			moves.push_back(pack_move(piece_t::bishop,
									  square_t::H7,
									  piece_t::pawn,
									  piece_t::empty,
									  square_t::G6));

			for (auto& move : moves)
			{
				EXPECT_TRUE(MoveGen::validate_move(
					pos, move, true));
			}
		}

		{
			ASSERT_TRUE(pos.reset(
					"4k3/8/8/8/8/8/8/4K3 w - - 0 1"));

			std::vector<int32> moves;

			moves.push_back(pack_move(piece_t::empty,
									  square_t::E1,
									  piece_t::king,
									  piece_t::empty,
									  square_t::G1));

			for (auto& move : moves)
			{
				EXPECT_FALSE(MoveGen::validate_move(
					pos, move, false));
			}
		}
	}
}
