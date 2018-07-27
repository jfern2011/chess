#include "gtest/gtest.h"

/*
 * Note: Include *after* gtest to protect against
 *       name conflicts with abort macros
 */

#include "src/MoveGen4.h"

using namespace Chess;

namespace
{
	TEST(MoveGen, position1)
	{
		Handle<std::ostream>
			stream(new std::ostream(std::cout.rdbuf()));

		Position pos(stream);

		ASSERT_TRUE(pos.reset(
			"r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq -"));

		int64 nodes = MoveGen::perft(pos, 3);

		EXPECT_EQ(nodes, 97862);
	}
}
