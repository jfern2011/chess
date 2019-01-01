#include "gtest/gtest.h"
#include "src/SearchPhase.h"

using namespace Chess;

namespace
{
	TEST(search, phase)
	{
		Handle<std::ostream>
			stream(new std::ostream(std::cout.rdbuf()));

		{
			/* 1 winning capture only */

			Position position(stream, "6k1/b4pP1/5P2/7N/5r2/7p/7P/7K w - - 0 1");

			SearchPhase phase;

			phase.init<phase_t::winning_captures>(position);

			int32 next;
			EXPECT_TRUE(phase.next_move<phase_t::winning_captures>(next));
			EXPECT_EQ(next, pack_move(piece_t::rook,
									  square_t::H5,
									  piece_t::knight,
									  piece_t::empty,
									  square_t::F4));

			EXPECT_FALSE(phase.next_move<phase_t::winning_captures>(next));

			phase.init<phase_t::non_captures>(position);

			EXPECT_TRUE(phase.next_move<phase_t::non_captures>(next));
			EXPECT_EQ(next, pack_move(piece_t::empty,
									  square_t::H5,
									  piece_t::knight,
									  piece_t::empty,
									  square_t::G3));

			EXPECT_FALSE(phase.next_move<phase_t::non_captures>(next));

			phase.init<phase_t::losing_captures>(position);

			EXPECT_FALSE(phase.next_move<phase_t::losing_captures>(next));
		}

		{
			/* 1 losing capture only */

			Position position(stream, "6k1/b4pP1/5P2/7N/5p2/7p/7P/7K w - - 0 1");

			SearchPhase phase;

			phase.init<phase_t::winning_captures>(position);

			int32 next;
			EXPECT_FALSE(phase.next_move<phase_t::winning_captures>(next));

			phase.init<phase_t::non_captures>(position);

			EXPECT_TRUE(phase.next_move<phase_t::non_captures>(next));
			EXPECT_EQ(next, pack_move(piece_t::empty,
									  square_t::H5,
									  piece_t::knight,
									  piece_t::empty,
									  square_t::G3));

			EXPECT_FALSE(phase.next_move<phase_t::non_captures>(next));

			phase.init<phase_t::losing_captures>(position);

			EXPECT_TRUE(phase.next_move<phase_t::losing_captures>(next));
			EXPECT_EQ(next, pack_move(piece_t::pawn,
									  square_t::H5,
									  piece_t::knight,
									  piece_t::empty,
									  square_t::F4));

			EXPECT_FALSE(phase.next_move<phase_t::losing_captures>(next));
		}

		{
			/* 1 winning, 1 losing capture */

			Position position(stream, "6k1/b4pP1/5r2/7N/5p2/7p/7P/7K w - - 0 1");

			SearchPhase phase;

			phase.init<phase_t::winning_captures>(position);

			int32 next;
			EXPECT_TRUE(phase.next_move<phase_t::winning_captures>(next));
			EXPECT_EQ(next, pack_move(piece_t::rook,
									  square_t::H5,
									  piece_t::knight,
									  piece_t::empty,
									  square_t::F6));

			EXPECT_FALSE(phase.next_move<phase_t::winning_captures>(next));

			phase.init<phase_t::non_captures>(position);

			EXPECT_TRUE(phase.next_move<phase_t::non_captures>(next));
			EXPECT_EQ(next, pack_move(piece_t::empty,
									  square_t::H5,
									  piece_t::knight,
									  piece_t::empty,
									  square_t::G3));

			EXPECT_FALSE(phase.next_move<phase_t::non_captures>(next));

			phase.init<phase_t::losing_captures>(position);

			EXPECT_TRUE(phase.next_move<phase_t::losing_captures>(next));
			EXPECT_EQ(next, pack_move(piece_t::pawn,
									  square_t::H5,
									  piece_t::knight,
									  piece_t::empty,
									  square_t::F4));

			EXPECT_FALSE(phase.next_move<phase_t::losing_captures>(next));
		}

		{
			/* No captures */

			Position position(stream, "6k1/b4pP1/5P2/7N/8/7p/7P/7K w - - 0 1");

			SearchPhase phase;

			phase.init<phase_t::winning_captures>(position);

			int32 next1, next2;
			EXPECT_FALSE(phase.next_move<phase_t::winning_captures>(next1));

			phase.init<phase_t::non_captures>(position);

			std::vector<int32> non_captures;
			non_captures.push_back(pack_move(piece_t::empty,
											 square_t::H5,
											 piece_t::knight,
											 piece_t::empty,
											 square_t::G3));

			non_captures.push_back(pack_move(piece_t::empty,
											 square_t::H5,
											 piece_t::knight,
											 piece_t::empty,
											 square_t::F4));

			EXPECT_TRUE(phase.next_move<phase_t::non_captures>(next1));

			EXPECT_TRUE(next1 == non_captures[0] ||
					    next1 == non_captures[1]);

			EXPECT_TRUE(phase.next_move<phase_t::non_captures>(next2));

			EXPECT_TRUE(next2 == non_captures[0] ||
					    next2 == non_captures[1]);

			EXPECT_NE(next1, next2);

			EXPECT_FALSE(phase.next_move<phase_t::non_captures>(next1));

			phase.init<phase_t::losing_captures>(position);

			EXPECT_FALSE(phase.next_move<phase_t::losing_captures>(next1));
		}

		{
			/* Winning and losing captures */

			Position position(stream, "6k1/5pP1/5P2/7N/5rp1/3pppPp/4Pb1P/7K w - - 0 1");

			SearchPhase phase;

			std::vector<int32> winning_captures;
			winning_captures.push_back(pack_move(piece_t::rook,
												 square_t::H5,
												 piece_t::knight,
												 piece_t::empty,
												 square_t::F4));

			winning_captures.push_back(pack_move(piece_t::rook,
												 square_t::G3,
												 piece_t::pawn,
												 piece_t::empty,
												 square_t::F4));

			phase.init<phase_t::winning_captures>(position);

			int32 next1, next2;
			EXPECT_TRUE(phase.next_move<phase_t::winning_captures>(next1));
			EXPECT_TRUE(phase.next_move<phase_t::winning_captures>(next2));

			EXPECT_TRUE(next1 == winning_captures[0] ||
					    next1 == winning_captures[1]);

			EXPECT_TRUE(next2 == winning_captures[0] ||
					    next2 == winning_captures[1]);

			EXPECT_NE(next1, next2);

			EXPECT_FALSE(phase.next_move<phase_t::winning_captures>(next1));

			phase.init<phase_t::non_captures>(position);

			EXPECT_FALSE(phase.next_move<phase_t::non_captures>(next1));

			phase.init<phase_t::losing_captures>(position);

			std::vector<int32> losing_captures;
			losing_captures.push_back(pack_move(piece_t::pawn,
												square_t::E2,
												piece_t::pawn,
												piece_t::empty,
												square_t::D3));

			losing_captures.push_back(pack_move(piece_t::pawn,
												square_t::E2,
												piece_t::pawn,
												piece_t::empty,
												square_t::F3));

			EXPECT_TRUE(phase.next_move<phase_t::losing_captures>(next1));
			EXPECT_TRUE(phase.next_move<phase_t::losing_captures>(next2));

			EXPECT_TRUE(next1 == losing_captures[0] ||
					    next1 == losing_captures[1]);

			EXPECT_TRUE(next2 == losing_captures[0] ||
					    next2 == losing_captures[1]);

			EXPECT_NE(next1, next2);

			EXPECT_FALSE(phase.next_move<phase_t::losing_captures>(next1));
		}

		{
			/* Winning captures only */

			Position position(stream, "6k1/5pP1/5P2/7N/5rp1/4p1Pp/4Pb1P/7K w - - 0 1");

			SearchPhase phase;

			std::vector<int32> winning_captures;
			winning_captures.push_back(pack_move(piece_t::rook,
												 square_t::H5,
												 piece_t::knight,
												 piece_t::empty,
												 square_t::F4));

			winning_captures.push_back(pack_move(piece_t::rook,
												 square_t::G3,
												 piece_t::pawn,
												 piece_t::empty,
												 square_t::F4));

			phase.init<phase_t::winning_captures>(position);

			int32 next1, next2;
			EXPECT_TRUE(phase.next_move<phase_t::winning_captures>(next1));
			EXPECT_TRUE(phase.next_move<phase_t::winning_captures>(next2));

			EXPECT_TRUE(next1 == winning_captures[0] ||
					    next1 == winning_captures[1]);

			EXPECT_TRUE(next2 == winning_captures[0] ||
					    next2 == winning_captures[1]);

			EXPECT_NE(next1, next2);

			EXPECT_FALSE(phase.next_move<phase_t::winning_captures>(next1));

			phase.init<phase_t::non_captures>(position);

			EXPECT_FALSE(phase.next_move<phase_t::non_captures>(next1));

			phase.init<phase_t::losing_captures>(position);

			EXPECT_FALSE(phase.next_move<phase_t::losing_captures>(next1));
		}

		{
			/* Losing captures only */

			Position position(stream, "2rkr2q/6pP/6PQ/6PP/8/8/3p4/3K4 w - - 0 1");

			SearchPhase phase;

			int32 next1, next2;
			phase.init<phase_t::winning_captures>(position);

			EXPECT_FALSE(phase.next_move<phase_t::winning_captures>(next1));

			phase.init<phase_t::non_captures>(position);

			EXPECT_FALSE(phase.next_move<phase_t::non_captures>(next1));

			phase.init<phase_t::losing_captures>(position);

			std::vector<int32> losing_captures;
			losing_captures.push_back(pack_move(piece_t::pawn,
												square_t::D1,
												piece_t::king,
												piece_t::empty,
												square_t::D2));

			losing_captures.push_back(pack_move(piece_t::pawn,
												square_t::H6,
												piece_t::queen,
												piece_t::empty,
												square_t::G7));

			
			EXPECT_TRUE(phase.next_move<phase_t::losing_captures>(next1));
			EXPECT_TRUE(phase.next_move<phase_t::losing_captures>(next2));

			EXPECT_TRUE(next1 == losing_captures[0] ||
					    next1 == losing_captures[1]);

			EXPECT_TRUE(next2 == losing_captures[0] ||
					    next2 == losing_captures[1]);

			EXPECT_NE(next1, next2);

			EXPECT_FALSE(phase.next_move<phase_t::losing_captures>(next1));
		}

		{
			/* Check evasions */

			Position position(stream, "3k4/8/8/8/8/8/2pb4/3K4 w - - 0 1");

			SearchPhase phase;

			int32 next1, next2, next3;
			phase.init<phase_t::check_evasions>(position);

			EXPECT_TRUE(phase.next_move<phase_t::check_evasions>(next1));
			EXPECT_TRUE(phase.next_move<phase_t::check_evasions>(next2));
			EXPECT_TRUE(phase.next_move<phase_t::check_evasions>(next3));

			std::vector<int32> evasions;
			evasions.push_back(pack_move(piece_t::bishop,
										 square_t::D1,
										 piece_t::king,
										 piece_t::empty,
										 square_t::D2));

			evasions.push_back(pack_move(piece_t::pawn,
										 square_t::D1,
										 piece_t::king,
										 piece_t::empty,
										 square_t::C2));

			evasions.push_back(pack_move(piece_t::empty,
										 square_t::D1,
										 piece_t::king,
										 piece_t::empty,
										 square_t::E2));

			EXPECT_TRUE(next1 == evasions[0] ||
					    next1 == evasions[1] ||
					    next1 == evasions[2]);

			EXPECT_TRUE(next2 == evasions[0] ||
					    next2 == evasions[1] ||
					    next2 == evasions[2]);

			EXPECT_TRUE(next3 == evasions[0] ||
					    next3 == evasions[1] ||
					    next3 == evasions[2]);

			EXPECT_NE(next1, next2);
			EXPECT_NE(next1, next3);
			EXPECT_NE(next2, next3);

			EXPECT_FALSE(phase.next_move<phase_t::check_evasions>(next1));

			/* Note all other phases are NOT called if
			 * we're in check
			 */
		}
	}
}
