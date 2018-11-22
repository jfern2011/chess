#include <sstream>
#include "gtest/gtest.h"

#include "src/see.h"

using namespace Chess;

namespace
{
	std::string record_to_string(const see_record& record)
	{
		size_t max_ind = std::max(record.captured.size(), record.moved.size());

		std::ostringstream out;
		out << "Record: " << record.captured.size() << " captures, "
			<< record.moved.size() << " moves:\n";

		for (size_t i = 0; i < max_ind; ++i)
		{
			if (i < record.moved.size())
				out << enum2piece(record.moved[i]) << "x";
			else
				out << "?x";

			if (i < record.captured.size())
				out << enum2piece(record.captured[i]) << "\n";
			else
				out << "?\n";
		}

		return out.str();
	}

	TEST(static_exchange, all)
	{
		Handle<std::ostream>
			stream(new std::ostream(std::cout.rdbuf()));

		Position pos(stream);

		{
			ASSERT_TRUE(pos.reset(
				"1k1r4/1pp4p/p7/4p3/8/P5P1/1PP4P/2K1R3 w - -"));

			see_record record;
			EXPECT_EQ(100, see(pos, pos.get_turn(), square_t::E5, record));

			ASSERT_TRUE(record.captured.size() == 1);
			ASSERT_TRUE(record.moved.size() == 1);

			EXPECT_EQ(piece_t::pawn, record.captured[0]);
			EXPECT_EQ(piece_t::rook, record.moved[0]);
		}

		{
			ASSERT_TRUE(pos.reset(
				"1k1r3q/1ppn3p/p4b2/4p3/8/P2N2P1/1PP1R1BP/2K1Q3 w - -"));

			see_record record, expected;
			EXPECT_EQ(-225, see(pos, pos.get_turn(), square_t::E5, record))
				<< record_to_string(record);

			expected.captured.push_back(piece_t::pawn);
			expected.moved.push_back(piece_t::knight);

			expected.captured.push_back(piece_t::knight);
			expected.moved.push_back(piece_t::knight);

			expected.captured.push_back(piece_t::knight);
			expected.moved.push_back(piece_t::rook);

			expected.captured.push_back(piece_t::rook);
			expected.moved.push_back(piece_t::bishop);

			expected.captured.push_back(piece_t::bishop);

			/* cut-off point if pruning */

			expected.moved.push_back(piece_t::queen);
			
			expected.captured.push_back(piece_t::queen);
			expected.moved.push_back(piece_t::queen);

			ASSERT_EQ(record.captured.size(), expected.captured.size())
				<< record_to_string(record);
			ASSERT_EQ(record.moved.size(), expected.moved.size())
				<< record_to_string(record);

			for (size_t i = 0; i < record.captured.size(); ++i)
			{
				EXPECT_EQ(record.captured[i], expected.captured[i]);
				EXPECT_EQ(record.moved[i], expected.moved[i]);
			}
		}

		{
			ASSERT_TRUE(pos.reset(
				"4b2k/7b/6B1/1B5b/2B5/1b1B4/4B3/3B3K w - - 0 1"));

			see_record record;
			EXPECT_EQ(-325, see(pos, pos.get_turn(), square_t::C6, record));

			ASSERT_EQ(record.captured.size(), 2) << record_to_string(record);
			ASSERT_EQ(record.moved.size(), 2) << record_to_string(record);

			EXPECT_EQ(piece_t::empty, record.captured[0]);
			EXPECT_EQ(piece_t::bishop, record.moved[0]);

			EXPECT_EQ(piece_t::bishop, record.captured[1]);
			EXPECT_EQ(piece_t::bishop, record.moved[1]);

			record.clear();
			EXPECT_EQ(-325, see(pos, pos.get_turn(), square_t::D5, record));

			ASSERT_EQ(record.captured.size(), 2) << record_to_string(record);
			ASSERT_EQ(record.moved.size(), 2) << record_to_string(record);

			EXPECT_EQ(piece_t::empty, record.captured[0]);
			EXPECT_EQ(piece_t::bishop, record.moved[0]);

			EXPECT_EQ(piece_t::bishop, record.captured[1]);
			EXPECT_EQ(piece_t::bishop, record.moved[1]);

			record.clear();
			EXPECT_EQ(0, see(pos, pos.get_turn(), square_t::E4, record));

			ASSERT_EQ(record.captured.size(), 3) << record_to_string(record);
			ASSERT_EQ(record.moved.size(), 3) << record_to_string(record);

			EXPECT_EQ(piece_t::empty, record.captured[0]);
			EXPECT_EQ(piece_t::bishop, record.moved[0]);

			EXPECT_EQ(piece_t::bishop, record.captured[1]);
			EXPECT_EQ(piece_t::bishop, record.moved[1]);

			EXPECT_EQ(piece_t::bishop, record.captured[2]);
			EXPECT_EQ(piece_t::bishop, record.moved[2]);

			record.clear();
			EXPECT_EQ(0, see(pos, pos.get_turn(), square_t::F3, record));

			ASSERT_EQ(record.captured.size(), 3) << record_to_string(record);
			ASSERT_EQ(record.moved.size(), 3) << record_to_string(record);

			EXPECT_EQ(piece_t::empty, record.captured[0]);
			EXPECT_EQ(piece_t::bishop, record.moved[0]);

			EXPECT_EQ(piece_t::bishop, record.captured[1]);
			EXPECT_EQ(piece_t::bishop, record.moved[1]);

			EXPECT_EQ(piece_t::bishop, record.captured[2]);
			EXPECT_EQ(piece_t::bishop, record.moved[2]);
		}
	}
}
