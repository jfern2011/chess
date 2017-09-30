#include <cstdio>
#include "movegen.h"

int main()
{
	const bool xboard  =  false;
	DataTables tables;
	Position pos(tables,xboard);
	MoveGen movegen(tables);

	if (!pos.reset("7k/3Q4/1N6/8/r3K3/1P6/3P4/8 w - - 0 1",
				  xboard))
		return 1;

	/*
	 * Moves to try:
	 *
	 * 1. bxa4
	 * 2. Nxa4
	 * 3. Qxa4
	 * 4. Nc4
	 * 5. Qd4
	 * 6. b4
	 * 7. d4
	 * 8. Ke3
	 */
	int moves[8] =
	{
		pack(ROOK,    B3, PAWN,   INVALID, A4),
		pack(ROOK,    B6, KNIGHT, INVALID, A4),
		pack(ROOK,    D7, QUEEN,  INVALID, A4),

		pack(INVALID, B6, KNIGHT, INVALID, C4),
		pack(INVALID, D7, QUEEN,  INVALID, D4),
		pack(INVALID, B3, PAWN,   INVALID, B4),
		pack(INVALID, D2, PAWN,   INVALID, D4),

		pack(INVALID, E4, KING,   INVALID, E3)
	};
	for (int i = 0; i < 8; i++)
	{
		std::cout << "Validating move "
			<< Util::printCoordinate(moves[i])
			<< "...";
		std::fflush(stdout);

		if (movegen.validateMove(pos, moves[i], true))
			std::cout << "legal" << std::endl;
		else
			std::cout << "illegal" << std::endl;
	}


	if (!pos.reset("8/3Rk3/8/3N4/8/5K2/8/8 b - - 0 1",
				   xboard))
		return 1;

	moves[0] = pack(ROOK,    E7, KING, INVALID, D7);
	moves[1] = pack(INVALID, E7, KING, INVALID, E8);

	std::printf("\n");

	for (int i = 0; i < 2; i++)
	{
		std::cout << "Validating move "
			<< Util::printCoordinate(moves[i])
			<< "...";
		std::fflush(stdout);

		if (movegen.validateMove(pos, moves[i], true))
			std::cout << "legal" << std::endl;
		else
			std::cout << "illegal" << std::endl;
	}


	if (!pos.reset("2n1k3/1P6/8/5pP1/3b4/2P5/P7/4K2R w K f6 0 1",
				   xboard))
		return 1;

	moves[0] = pack(INVALID, A2, PAWN, INVALID, A3);
	moves[1] = pack(INVALID, A2, PAWN, INVALID, A4);
	moves[2] = pack(BISHOP,  C3, PAWN, INVALID, D4);
	moves[3] = pack(INVALID, B7, PAWN, QUEEN,   B8);
	moves[4] = pack(KNIGHT,  B7, PAWN, BISHOP,  C8);
	moves[5] = pack(PAWN,    G5, PAWN, INVALID, F6);
	moves[6] = pack(INVALID, E1, KING, INVALID, G1); // Invalid

	std::printf("\n");

	for (int i = 0; i < 7; i++)
	{
		std::cout << "Validating move "
			<< Util::printCoordinate(moves[i])
			<< "...";
		std::fflush(stdout);

		if (movegen.validateMove(pos, moves[i], false))
			std::cout << "legal" << std::endl;
		else
			std::cout << "illegal" << std::endl;
	}


	if (!pos.reset("2n1k3/1P5p/8/1N3pP1/8/2P5/P2r4/4K2R w K - 0 1",
				   xboard))
		return 1;

	moves[0] = pack(INVALID, E1, KING,   INVALID, G1);
	moves[1] = pack(ROOK,    E1, KING,   INVALID, D2);
	moves[2] = pack(INVALID, B5, KNIGHT, INVALID, D6);
	moves[3] = pack(PAWN,    H1, ROOK,   INVALID, H7);

	std::printf("\n");

	for (int i = 0; i < 4; i++)
	{
		std::cout << "Validating move "
			<< Util::printCoordinate(moves[i])
			<< "...";
		std::fflush(stdout);

		if (movegen.validateMove(pos, moves[i], false))
			std::cout << "legal" << std::endl;
		else
			std::cout << "illegal" << std::endl;
	}

	return 0;
}
