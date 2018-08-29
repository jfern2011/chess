#include "perft.h"
#include "src/MoveGen4.h"

namespace Chess
{
	/**
	 * Performance test. Walks the move generation tree of strictly
	 * legal moves, counting up the number of resulting positions
	 *
	 * @param[in] pos   The position to run the test on
	 * @param[in] depth The maximum depth to traverse
	 *
	 * @return The number of possible positions up to and including
	 *         \a depth
	 */
	int64 perft(Position& pos, int depth)
	{
		int32 moves[max_moves];

		/*
		 * Generate all possible captures and non-captures
		 */
		size_t n_moves = 0;

		if (pos.in_check(pos.get_turn()))
			n_moves  = MoveGen::generate_check_evasions(pos, moves);
		else
		{
			n_moves  = MoveGen::generate_captures(pos, moves);
			n_moves +=
				MoveGen::generate_noncaptures(pos, &moves[n_moves]);
		}

		if (depth <= 1)
			return n_moves;

		int64 nodes = 0;

		for (register size_t i = 0; i < n_moves; i++)
		{
			pos.make_move(moves[i]);

			nodes += perft( pos, depth-1 );

			pos.unmake_move(moves[i]);
		}

		return nodes;
	}
}
