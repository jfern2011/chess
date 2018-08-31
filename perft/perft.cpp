#include <iostream>

#include "perft.h"
#include "src/MoveGen4.h"

namespace Chess
{
	/**
	 * Computes the number of nodes per legal move. This is similar
	 * to perft(), but breaks down the node count by move
	 *
	 * @param[in] pos   The position to run the test on
	 * @param[in] depth The maximum depth to traverse
	 *
	 * @return The number of possible positions up to and including
	 *         \a depth
	 */
	int64 divide(Position& pos, int depth)
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

		int64 nodes = 0, cumnodes = 0;

		for (size_t i = 0; i < n_moves; i++)
		{
			pos.make_move(moves[i]);

			nodes = ( depth <= 1 ) ? 1 : perft(pos, depth-1);

			const square_t from = extract_from(moves[i]);
			const square_t to   = extract_to  (moves[i]);

			std::cout  << square_str[from] << square_str[to];

			switch (extract_promote(moves[i]))
			{
			case piece_t::knight:
				std::cout << 'N'; break;
			case piece_t::rook:
				std::cout << 'R'; break;
			case piece_t::bishop:
				std::cout << 'B'; break;
			case piece_t::queen:
				std::cout << 'Q'; break;
			default: break;
			}
			
			std::cout << ": " << nodes << std::endl;

			pos.unmake_move(moves[i]);
			cumnodes += nodes;
		}

		return cumnodes;
	}

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
