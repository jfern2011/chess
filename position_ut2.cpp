#include <cstdlib>

#include "abort.h"
#include "clock.h"
#include "movegen2.h"
#include "position2.h"

MoveGen movegen(data_tables);

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
int perft2(Position& pos, int depth)
{
	uint32 moves[MAX_MOVES];

	/*
	 * Generate strictly legal moves:
	 */
	uint32 nMoves;

	if (pos.in_check(pos.get_turn()))
		nMoves =
		   movegen.generate_check_evasions(pos, pos.get_turn(), moves);
	else
		nMoves = movegen.generate_legal_moves(pos, pos.get_turn(), moves);

	if (depth <= 1)
		return nMoves;

	int nodes = 0;

	for (register int i = 0; i < nMoves; i++)
	{
		pos.make_move(moves[i]);

		nodes += perft2(pos, depth-1);

		pos.unmake_move(moves[i]);
	}

	return nodes;
}

int main(int argc, char** argv)
{
	DataTables tables;
	Position pos(tables,
		         //"r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq -",
		         false);

	int64 start = Clock::get_monotonic_time();

	int nodes = perft2(pos, 6);

	int64 dt = Clock::get_monotonic_time() - start;

	std::printf("total moves = %d, time = %lld \n",
		nodes, dt);
	std::fflush(stdout);
/*
	AbortIfNot(pos.make_move(0),
		EXIT_FAILURE);
*/
	return EXIT_SUCCESS;
}
