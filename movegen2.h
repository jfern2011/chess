#ifndef __MOVEGEN__
#define __MOVEGEN__

#include "position2.h"


/**
 **********************************************************************
 *
 * @class MoveGen
 *
 * Generates captures, non-captures, checks, and check evasions, all
 * which are strictly legal
 *
 **********************************************************************
 */
class MoveGen
{

public:

	MoveGen(const DataTables& tables);

	~MoveGen();

	uint32 generate_captures(const Position& pos, int to_move,
							 uint32* captures) const;

	uint32 generate_check_evasions(const Position& pos, int to_move,
								   uint32* evasions) const;
	
	uint32 generate_checks(const Position& pos, int to_move,
						   uint32* checks) const;

	uint32 generate_legal_moves(const Position& pos, int to_move,
								uint32* moves) const;

	uint32 generate_non_captures(const Position& pos, int to_move,
								 uint32* moves) const;

	static bool isLegal(const Position& pos, int move);

	int64 perft(Position& pos, int depth) const;

	bool validateMove(const Position& pos, int move,
					  bool in_check) const;

private:

	const DataTables& _tables;
};

#endif
