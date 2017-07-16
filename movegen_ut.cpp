#include <iostream>
#include <string>

#include "movegen.h"

int main(int argc, char** argv)
{
	DataTables tables;
	MoveGen movegen(tables);

	std::string fen = "8/6p1/4p3/2k5/5K2/3P4/1P6/8 b - - 1 1";

	Position pos(tables, fen, false);

	uint32 moves[128];

	uint32* end =
			movegen.generateChecks(pos, pos.getTurn(), moves);

	int n_moves = end - moves;

	for (int i = 0; i < n_moves; i++)
	{
		std::cout << "=====\n";
		   Util::printMove(moves[i]);
	}

	std::cout << "\n...........\n\n";

	fen = "8/4K1pr/8/8/8/8/RP1k4/8 b - - 1 1";
	pos.reset(fen, false);
	end = movegen.generateChecks( pos, pos.getTurn(), moves );

	n_moves = end - moves;

	for (int i = 0; i < n_moves; i++)
	{
		std::cout << "=====\n";
		   Util::printMove(moves[i]);
	}

	std::cout << "\n...........\n\n";

	fen = "4n3/1K4nr/8/8/8/8/RN4k1/3N4 b - - 1 1";
	pos.reset(fen, false);
	end = movegen.generateChecks( pos, pos.getTurn(), moves );

	n_moves = end - moves;

	for (int i = 0; i < n_moves; i++)
	{
		std::cout << "=====\n";
		   Util::printMove(moves[i]);
	}

	std::cout << "\n...........\n\n";

	fen = "4k2r/8/8/8/8/8/8/5K2 b k - 1 1";
	pos.reset(fen, false);
	end = movegen.generateChecks( pos, pos.getTurn(), moves );

	n_moves = end - moves;

	for (int i = 0; i < n_moves; i++)
	{
		std::cout << "=====\n";
		   Util::printMove(moves[i]);
	}

	std::cout << "\n...........\n\n";

	fen = "1K2k2r/8/8/8/8/8/8/8 b k - 1 1";
	pos.reset(fen, false);
	end = movegen.generateChecks( pos, pos.getTurn(), moves );

	n_moves = end - moves;

	for (int i = 0; i < n_moves; i++)
	{
		std::cout << "=====\n";
		   Util::printMove(moves[i]);
	}

	std::cout << "\n...........\n\n";

	fen = "8/8/1b6/2k5/8/8/8/6K1 b - - 1 1";
	pos.reset(fen, false);
	end = movegen.generateChecks( pos, pos.getTurn(), moves );

	n_moves = end - moves;

	for (int i = 0; i < n_moves; i++)
	{
		std::cout << "=====\n";
		   Util::printMove(moves[i]);
	}

	std::cout << "\n...........\n\n";

	fen = "8/6k1/1b6/2n5/8/8/8/6K1 b - - 1 1";
	pos.reset(fen, false);
	end = movegen.generateChecks( pos, pos.getTurn(), moves );

	n_moves = end - moves;

	for (int i = 0; i < n_moves; i++)
	{
		std::cout << "=====\n";
		   Util::printMove(moves[i]);
	}

	std::cout << "\n...........\n\n";

	fen = "8/8/1bp5/2rp2k1/8/8/2P5/6K1 b - - 1 1";
	pos.reset(fen, false);
	end = movegen.generateChecks( pos, pos.getTurn(), moves );

	n_moves = end - moves;

	for (int i = 0; i < n_moves; i++)
	{
		std::cout << "=====\n";
		   Util::printMove(moves[i]);
	}

	std::cout << "\n...........\n\n";

	fen = "2r5/2b5/8/6k1/8/2K5/8/8 b - - 1 1";
	pos.reset(fen, false);
	end = movegen.generateChecks( pos, pos.getTurn(), moves );

	n_moves = end - moves;

	for (int i = 0; i < n_moves; i++)
	{
		std::cout << "=====\n";
		   Util::printMove(moves[i]);
	}

	std::cout << "\n...........\n\n";

	fen = "2r5/2b5/8/P3P1k1/8/2K3b1/8/8 b - - 1 1";
	pos.reset(fen, false);
	end = movegen.generateChecks( pos, pos.getTurn(), moves );

	n_moves = end - moves;

	for (int i = 0; i < n_moves; i++)
	{
		std::cout << "=====\n";
		   Util::printMove(moves[i]);
	}

	std::cout << "\n...........\n\n";

	fen = "2q5/2b5/3P4/P5k1/8/2K5/8/8 b - - 1 1";
	pos.reset(fen, false);
	end = movegen.generateChecks( pos, pos.getTurn(), moves );

	n_moves = end - moves;

	for (int i = 0; i < n_moves; i++)
	{
		std::cout << "=====\n";
		   Util::printMove(moves[i]);
	}

	return 0;
}
